import libanimax from './libanimax';
import path from 'path';
import sharp from 'sharp';
import fs from 'fs';

interface RenderLottieTask {
  lottieJSONPath: string;
  width: number;
  height: number;
  frames: number[];
  format: string;
  outputDir: string;
}

interface RenderLottieResult {
  path: string;
  frame: number;
  width: number;
  height: number;
  format: string;
}

const renderLottieJSON = async (
  renderTask: RenderLottieTask
): Promise<RenderLottieResult[]> => {
  const {
    lottieJSONPath,
    width,
    height,
    frames,
    format,
    outputDir,
  } = renderTask;
  if (!path.isAbsolute(lottieJSONPath)) {
    throw new Error('Lottie JSON path must be absolute');
  }
  const animaxRenderTask = libanimax.createRenderTask(
    lottieJSONPath,
    width,
    height
  );
  await fs.promises.mkdir(outputDir, { recursive: true });
  try {
    const results: RenderLottieResult[] = [];
    for (const frame of frames) {
      const buffer = libanimax.render(animaxRenderTask, frame);
      const outputPath = path.join(outputDir, `${frame}.${format}`);
      await sharp(buffer, {
        raw: {
          width,
          height,
          channels: 4,
        },
      }).toFile(outputPath);
      results.push({
        path: outputPath,
        frame,
        width,
        height,
        format,
      });
    }
    return results;
  } finally {
    libanimax.destroyRenderTask(animaxRenderTask);
  }
};

type RenderLottieJSON = typeof renderLottieJSON;

export {
  renderLottieJSON,
  RenderLottieJSON,
  RenderLottieTask,
  RenderLottieResult,
};
