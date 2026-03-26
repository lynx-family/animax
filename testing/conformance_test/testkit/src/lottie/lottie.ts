import * as path from 'path';
import sharp from 'sharp';
import * as fs from 'fs';
import {
  LOTTIE_JSON_FILENAME,
  UNTRANSFORMED_LOTTIE_JSON_FILENAME,
} from '../constant';
import axios from 'axios';

interface LottieImageAsset {
  id: string;
  w: number;
  h: number;
  p: string;
  u?: string;
}

interface LottieFont {
  fName: string;
  fPath: string;
}

interface LottieVideoAsset {
  id: string;
  x: number;
  y: number;
  w: number;
  h: number;
  ax: number;
  ay: number;
  aw: number;
  ah: number;
  u: string;
  p: string;
  sz: number;
}

interface Lottie {
  w: number;
  h: number;
  ip: number;
  op: number;
  fr: number;
  assets?: LottieImageAsset[];
  fonts?: {
    list: LottieFont[];
  };
  videos?: LottieVideoAsset[];
  layers: object[];
}

interface LottieAssetTransformContext {
  originalLottieURI: string;
  lottiePath: string;
  lottie: Lottie;
}

interface LottieAssetTransformFunctions {
  image?: (
    asset: LottieImageAsset,
    context: LottieAssetTransformContext
  ) => LottieImageAsset | Promise<LottieImageAsset>;
  font?: (
    asset: LottieFont,
    context: LottieAssetTransformContext
  ) => LottieFont | Promise<LottieFont>;
  video?: (
    asset: LottieVideoAsset,
    context: LottieAssetTransformContext
  ) => LottieVideoAsset | Promise<LottieVideoAsset>;
}

const isHttpURL = (str: string) => {
  try {
    const url = new URL(str);
    return url.protocol === 'http:' || url.protocol === 'https:';
  } catch {
    return false;
  }
};

const isBase64URL = (str: string) => {
  return /^data:image\/.+;base64,/.test(str);
};

const imageURLFromAsset = (
  lottiePath: string,
  asset: LottieImageAsset
): string => {
  const { u = '', p } = asset;

  if (isBase64URL(p)) {
    return p;
  }

  if (path.isAbsolute(lottiePath) && !isHttpURL(p)) {
    // Lottie JSON is a local file
    const result = path.join(u, p);
    if (path.isAbsolute(result)) {
      return result;
    }
    return path.join(path.dirname(lottiePath), result);
  }

  let resolvedPath = path.join(u, p);

  if (!isHttpURL(resolvedPath)) {
    resolvedPath = path.join(path.dirname(lottiePath), resolvedPath);
  }

  if (!isHttpURL(resolvedPath) && !path.isAbsolute(resolvedPath)) {
    throw new Error(
      `Cannot resolve image path for image: ${JSON.stringify(asset)}`
    );
  }

  return resolvedPath;
};

const imageAssetTransform = (generatedImages: string[]) => async (
  asset: LottieImageAsset,
  context: LottieAssetTransformContext
) => {
  const imageURL = imageURLFromAsset(context.originalLottieURI, asset);
  let imageBuffer: Buffer;
  if (isBase64URL(imageURL)) {
    const base64Data = imageURL.replace(/^data:image\/\w+;base64,/, '');
    imageBuffer = Buffer.from(base64Data, 'base64');
  } else if (path.isAbsolute(imageURL)) {
    imageBuffer = await fs.promises.readFile(imageURL);
  } else if (isHttpURL(imageURL)) {
    const imageResponse = await axios.get(imageURL, {
      responseType: 'arraybuffer',
    });
    if (imageResponse.status !== 200) {
      throw new Error(
        `Failed to fetch the image: ${imageURL}, ${imageResponse.statusText}`
      );
    }
    imageBuffer = Buffer.from(imageResponse.data); // Get the image buffer
  } else {
    throw new Error(`Invalid image URL: ${imageURL}`);
  }

  const imageFilename = asset.id + '.png';
  const imagePath = path.join(path.dirname(context.lottiePath), imageFilename);
  await sharp(imageBuffer).png().toFile(imagePath);
  generatedImages.push(imagePath);
  const newAsset = { ...asset, p: imageFilename };
  if (newAsset.u) {
    // remove the folder path from the asset.
    delete newAsset.u;
  }
  return newAsset;
};

const transformAssets = async (
  transform: LottieAssetTransformFunctions,
  context: LottieAssetTransformContext
): Promise<Lottie> => {
  const lottie = { ...context.lottie };
  const { assets = [], fonts = { list: [] }, videos = [] } = lottie;
  if (assets.length > 0 && transform.image == undefined) {
    throw new Error(
      'Image assets found but no image asset transform function provided'
    );
  }
  if (fonts.list.length > 0 && transform.font == undefined) {
    throw new Error(
      'Font assets found but no font asset transform function provided'
    );
  }
  if (videos.length > 0 && transform.video == undefined) {
    throw new Error(
      'Video assets found but no video asset transform function provided'
    );
  }

  const idTransform = <T>(asset: T) => asset;

  const imageTransform = transform.image ? transform.image : idTransform;
  const fontTransform = transform.font ? transform.font : idTransform;
  const videoTransform = transform.video ? transform.video : idTransform;

  const transformedAssetsTask = assets
    .filter((asset) => 'p' in asset)
    .map((img) => Promise.resolve(imageTransform(img, context)));
  const otherAssets = assets.filter((asset) => !('p' in asset));

  const transformedFontsTask = fonts.list.map((font) =>
    Promise.resolve(fontTransform(font, context))
  );
  const transformedVideosTask = videos.map((video) =>
    Promise.resolve(videoTransform(video, context))
  );

  const transformedAssets = await Promise.all(transformedAssetsTask);
  const mergeAssets = [...otherAssets, ...transformedAssets];

  const transformedFonts = await Promise.all(transformedFontsTask);
  const transformedVideos = await Promise.all(transformedVideosTask);

  return {
    ...lottie,
    assets: mergeAssets,
    fonts: { ...lottie.fonts, list: transformedFonts },
    videos: transformedVideos,
  };
};

interface DownloadLottieOptions {
  url?: string;
  filePath?: string;
  outputDir: string;
}

const prepareLottie = async (options: DownloadLottieOptions) => {
  const { url, filePath, outputDir } = options;
  let rawLottieFilePath = filePath;
  try {
    if (url == undefined && filePath == undefined) {
      throw new Error('Either url or path must be provided');
    }

    await fs.promises.mkdir(outputDir, { recursive: true });
    rawLottieFilePath = path.join(
      outputDir,
      UNTRANSFORMED_LOTTIE_JSON_FILENAME
    );
    if (url != undefined && filePath == undefined) {
      const lottie = await axios.get(url).then((response) => response.data);
      await fs.promises.writeFile(
        rawLottieFilePath,
        JSON.stringify(lottie, null, 2)
      );
    } else if (filePath != undefined) {
      const lottie = await fs.promises.readFile(filePath, 'utf-8');
      await fs.promises.writeFile(rawLottieFilePath, lottie);
    }

    if (rawLottieFilePath == undefined) {
      throw new Error('Failed to download lottie.json');
    }

    const lottieString = await fs.promises.readFile(rawLottieFilePath, 'utf8');
    const lottie = (JSON.parse(lottieString) as unknown) as Lottie;
    const generatedImages: string[] = [];
    const transformedLottie = await transformAssets(
      {
        image: imageAssetTransform(generatedImages),
      },
      {
        originalLottieURI: url ? url : rawLottieFilePath,
        lottiePath: rawLottieFilePath,
        lottie,
      }
    );

    const transformedLottiePath = path.join(outputDir, LOTTIE_JSON_FILENAME);
    await fs.promises.writeFile(
      transformedLottiePath,
      JSON.stringify(transformedLottie, null, 2)
    );

    return {
      lottie: transformedLottiePath,
      assets: generatedImages,
    };
  } finally {
    if (rawLottieFilePath != undefined) {
      await fs.promises.rm(rawLottieFilePath);
    }
  }
};

export { prepareLottie, Lottie };
