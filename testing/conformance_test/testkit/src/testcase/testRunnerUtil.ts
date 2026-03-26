import { LottieTestPlanWithPath } from './testcase';
import { RenderLottieResult, RenderLottieTask } from '../render/render';
import path from 'path';
import sharp from 'sharp';
import fs from 'fs';
import {
  TESTRUNNER_DIFF_IMAGE_DIRNAME,
  TESTRUNNER_GENERATED_CONTENT_DIRNAME,
} from '../constant';

interface CompareTask {
  testPlanName: string;
  frame: number;
  imagePath: string;
  baselineImagePath: string;
}

type RenderTaskWithTestPlanName = RenderLottieTask & { name: string };

const generateRenderTasks = (
  plans: LottieTestPlanWithPath[],
  output: string
): RenderTaskWithTestPlanName[] => {
  return plans.map(
    (plan: LottieTestPlanWithPath): RenderTaskWithTestPlanName => {
      return {
        ...plan,
        format: 'png',
        outputDir: path.join(
          output,
          TESTRUNNER_GENERATED_CONTENT_DIRNAME,
          plan.name
        ),
      };
    }
  );
};

const generateCompareTask = (
  renderTasks: RenderTaskWithTestPlanName[],
  renderResults: RenderLottieResult[][],
  plans: LottieTestPlanWithPath[]
) => {
  if (renderResults.length !== renderTasks.length) {
    throw new Error('renderResults length not equal to renderTasks length');
  }

  const compareTasks: CompareTask[] = [];

  for (let i = 0; i < renderResults.length; ++i) {
    const plan = plans[i];
    const result = renderResults[i];

    result.map((r) => {
      compareTasks.push({
        testPlanName: plan.name,
        frame: r.frame,
        imagePath: r.path,
        baselineImagePath: plan.baselineImagePaths.get(r.frame)!,
      });
    });
  }

  return compareTasks;
};

interface ImageDiffResult {
  baseline: string;
  generated: string;
  same: boolean;
  message: string;
  diff: string | undefined;
}

type ImageDiffResultWithNameFrame = ImageDiffResult & {
  testPlanName: string;
  frame: number;
};

type DiffFunction = (lhs: string, rhs: string) => Promise<ImageDiffResult>;

const runCompareTasks = async (tasks: CompareTask[], diff: DiffFunction) =>
  Promise.all(
    tasks.map((task) =>
      diff(task.baselineImagePath, task.imagePath).then(
        (diffResult): ImageDiffResultWithNameFrame => {
          return {
            ...diffResult,
            testPlanName: task.testPlanName,
            frame: task.frame,
          };
        }
      )
    )
  );

const imageDiffSame = async (
  baseline: string,
  generated: string
): Promise<ImageDiffResult> => {
  const [
    { data: lhsData, info: lhsInfo },
    { data: rhsData, info: rhsInfo },
  ] = await Promise.all(
    [baseline, generated].map((p) =>
      sharp(p).raw().toBuffer({ resolveWithObject: true })
    )
  );

  if (lhsInfo.width !== rhsInfo.width || lhsInfo.height !== rhsInfo.height) {
    return {
      baseline: baseline,
      generated: generated,
      same: false,
      message: `image size not equal, ${lhsInfo.width}x${lhsInfo.height} vs ${rhsInfo.width}x${rhsInfo.height}`,
      diff: undefined,
    };
  } else if (lhsInfo.channels !== rhsInfo.channels) {
    return {
      baseline: baseline,
      generated: generated,
      same: false,
      message: `image channels not equal, ${lhsInfo.channels} vs ${rhsInfo.channels}`,
      diff: undefined,
    };
  }
  const width = lhsInfo.width;
  const height = lhsInfo.height;
  const diffData = Buffer.alloc(width * height * 4);
  let accumulatedDiff = 0;

  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      const idx = (y * width + x) * 4;
      const rA = lhsData[idx];
      const gA = lhsData[idx + 1];
      const bA = lhsData[idx + 2];
      const aA = lhsData[idx + 3];

      const rB = rhsData[idx];
      const gB = rhsData[idx + 1];
      const bB = rhsData[idx + 2];
      const aB = rhsData[idx + 3];

      const diff =
        Math.abs(rA - rB) +
        Math.abs(gA - gB) +
        Math.abs(bA - bB) +
        Math.abs(aA - aB);
      const isDifferent = diff !== 0;
      accumulatedDiff += diff;

      if (isDifferent) {
        diffData[idx] = 255; // R
        diffData[idx + 1] = 0; // G
        diffData[idx + 2] = 0; // B
        diffData[idx + 3] = 255; // A (fully opaque)
      } else {
        // If not different, copy the original pixel from imageA or imageB
        diffData[idx] = rA;
        diffData[idx + 1] = gA;
        diffData[idx + 2] = bA;
        diffData[idx + 3] = aA;
      }
    }
  }

  if (accumulatedDiff === 0) {
    return {
      baseline: baseline,
      generated: generated,
      same: true,
      message: 'image is same',
      diff: undefined,
    };
  }

  const diffImagesDir = path.join(
    path.dirname(generated),
    TESTRUNNER_DIFF_IMAGE_DIRNAME
  );
  await fs.promises.mkdir(diffImagesDir, { recursive: true });
  const diffPath = path.join(diffImagesDir, `${path.basename(baseline)}`);

  await sharp(diffData, {
    raw: {
      width,
      height,
      channels: 4,
    },
  })
    .png()
    .toFile(diffPath);

  return {
    baseline: baseline,
    generated: generated,
    same: false,
    message: 'image is different',
    diff: diffPath,
  };
};

interface TestReport {
  testName: string;
  failedFrames: {
    frame: number;
    diff: string;
  }[];
}

const groupBy = <T>(
  array: T[],
  predicate: (value: T, index: number, array: T[]) => string
) =>
  array.reduce((acc, value, index, array) => {
    (acc[predicate(value, index, array)] ||= []).push(value);
    return acc;
  }, {} as { [key: string]: T[] });

const generatedFailedTestReports = (
  diffResults: ImageDiffResultWithNameFrame[],
  output: string
) => {
  const groupedTestResult = groupBy(
    diffResults.filter((r) => !r.same),
    (r) => r.testPlanName
  );
  const failedTestReports = Object.entries(groupedTestResult).map(
    ([name, result]): TestReport => {
      return {
        testName: name,
        failedFrames: result.map((r) => {
          return {
            frame: r.frame,
            diff: r.diff ? path.relative(output, r.diff) : '',
          };
        }),
      };
    }
  );
  return failedTestReports;
};

export {
  generateRenderTasks,
  generateCompareTask,
  runCompareTasks,
  imageDiffSame,
  generatedFailedTestReports,
};
