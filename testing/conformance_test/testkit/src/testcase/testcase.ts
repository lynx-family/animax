import { RenderLottieServer } from '../render/server';
import { Lottie, prepareLottie } from '../lottie/lottie';
import fs from 'fs';
import path from 'path';
import { RenderLottieTask } from '../render/render';
import {
  TESTPLAN_BASELINE_IMAGE_MIN_DIMENSION,
  LOTTIE_JSON_FILENAME,
  TESTPLAN_BASELINE_IMAGE_DIRNAME,
  TESTPLAN_BASELINE_IMAGE_MAX_DIMENSION,
  TESTPLAN_BASELINE_IMAGE_PER_TASK,
  TESTPLAN_JSON_FILENAME,
} from '../constant';

interface LottieTestPlanOptions {
  name: string;
  width: number;
  height: number;
}

interface LottieTestPlan {
  name: string;
  width: number;
  height: number;
  frames: number[];
}

const decideBaselineImagesDimensions = (width: number, height: number) => {
  const aspectRatio = width / height;

  if (width < TESTPLAN_BASELINE_IMAGE_MIN_DIMENSION) {
    width = TESTPLAN_BASELINE_IMAGE_MIN_DIMENSION;
    height = Math.round(width / aspectRatio);
  }

  if (height < TESTPLAN_BASELINE_IMAGE_MIN_DIMENSION) {
    height = TESTPLAN_BASELINE_IMAGE_MIN_DIMENSION;
    width = Math.round(height * aspectRatio);
  }

  // Keep aspect ratio and make sure that the width and height are less than 500
  if (width > TESTPLAN_BASELINE_IMAGE_MAX_DIMENSION) {
    width = TESTPLAN_BASELINE_IMAGE_MAX_DIMENSION;
    height = Math.round(width / aspectRatio);
  }

  if (height > TESTPLAN_BASELINE_IMAGE_MAX_DIMENSION) {
    height = TESTPLAN_BASELINE_IMAGE_MAX_DIMENSION;
    width = Math.round(height * aspectRatio);
  }

  return {
    width,
    height,
  };
};

const createTestPlan = (
  lottie: Lottie,
  options: LottieTestPlanOptions
): LottieTestPlan => {
  const startFrame = Math.floor(lottie.ip);
  const endFrame = Math.floor(lottie.op);
  const totalFrames = endFrame - startFrame;
  const stride = Math.floor(totalFrames / TESTPLAN_BASELINE_IMAGE_PER_TASK);
  const frames = [];
  for (let i = startFrame; i < endFrame; i += stride) {
    frames.push(i);
  }
  return {
    name: options.name,
    frames,
    ...decideBaselineImagesDimensions(options.width, options.height),
  };
};

const writeTestPlanToJSON = (testPlan: LottieTestPlan, outputDir: string) => {
  const testPlanPath = path.join(outputDir, TESTPLAN_JSON_FILENAME);
  fs.writeFileSync(testPlanPath, JSON.stringify(testPlan, null, 2));
  return testPlanPath;
};

const generateBaselineImages = async (
  lottieJSONPath: string,
  testPlan: LottieTestPlan,
  outputDir: string,
  renderServer: RenderLottieServer
) => {
  const renderTask: RenderLottieTask = {
    lottieJSONPath,
    outputDir: path.join(outputDir, TESTPLAN_BASELINE_IMAGE_DIRNAME),
    frames: testPlan.frames,
    format: 'png',
    width: testPlan.width,
    height: testPlan.height,
  };
  const results = await renderServer.renderLottieJSON(renderTask);
  return results.map((r) => r.path);
};

interface generateTestPlanOptions {
  lottieJSONPath: string;
  name: string;
  outputDir: string;
  server: RenderLottieServer;
  width: number;
  height: number;
}

const generateTestPlan = async ({
  lottieJSONPath,
  name,
  outputDir,
  server,
  width,
  height,
}: generateTestPlanOptions) => {
  let createdFiles: string[] = [];
  try {
    const {
      lottie: testCaseLottiePath,
      assets: imagesPath,
    } = await prepareLottie({
      filePath: lottieJSONPath,
      outputDir,
    });
    const lottie = JSON.parse(
      fs.readFileSync(testCaseLottiePath, 'utf8')
    ) as Lottie;
    const testPlan = createTestPlan(lottie, { name, width, height });
    const testPlanPath = writeTestPlanToJSON(testPlan, outputDir);
    const baselineImages = await generateBaselineImages(
      testCaseLottiePath,
      testPlan,
      outputDir,
      server
    );
    createdFiles = [
      testCaseLottiePath,
      ...imagesPath,
      testPlanPath,
      ...baselineImages,
    ];
  } catch (e) {
    console.error(e);
    await Promise.all([createdFiles.map((f) => fs.promises.rm(f))]);
    throw e;
  }
};

type LottieTestPlanWithPath = LottieTestPlan & {
  outputPath: string;
  lottieJSONPath: string;
  baselineImagePaths: Map<number, string>;
};

const getDirectories = (basePath: string): string[] => {
  const entries = fs.readdirSync(basePath, { withFileTypes: true });

  const directories = entries
    .filter((entry) => entry.isDirectory())
    .map((entry) => path.join(basePath, entry.name));

  return directories;
};

const deriveTestPlanFromDir = async (
  dir: string
): Promise<LottieTestPlanWithPath> => {
  const testPlanPath = path.join(dir, TESTPLAN_JSON_FILENAME);
  const testPlan: LottieTestPlan = await fs.promises
    .readFile(testPlanPath, 'utf8')
    .then(JSON.parse);
  const baselineImagePaths = new Map<number, string>();
  const baselineImageDir = path.join(dir, TESTPLAN_BASELINE_IMAGE_DIRNAME);
  const baselineImages = await fs.promises.readdir(baselineImageDir);
  baselineImages.forEach((image) => {
    const frame = parseInt(image.split('.')[0]);
    baselineImagePaths.set(frame, path.join(baselineImageDir, image));
  });
  return {
    ...testPlan,
    outputPath: dir,
    lottieJSONPath: path.join(dir, LOTTIE_JSON_FILENAME),
    baselineImagePaths,
  };
};

const gatherTestPlans = async (
  path: string
): Promise<LottieTestPlanWithPath[]> => {
  const testplanDirs = getDirectories(path);
  return Promise.all(testplanDirs.map((dir) => deriveTestPlanFromDir(dir)));
};

export {
  generateTestPlan,
  gatherTestPlans,
  LottieTestPlan,
  LottieTestPlanWithPath,
};
