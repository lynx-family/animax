import { Option, program } from 'commander';

import { RenderLottieTask } from './render';
import { generateAnimatedWebp } from './webpmux';
import { Lottie, prepareLottie } from '../lottie/lottie';
import { generateTestPlan } from '../testcase/testcase';
import fs from 'fs';
import path from 'path';
import {
  CLI_ANIMATED_WEBP_FILENAME,
  RENDERSERVER_MAX_FRAME_PER_TASK,
} from '../constant';
import { RenderLottieServer } from './server';

interface ProgramOptions {
  input: string;
  width?: number;
  height?: number;
  output: string;
  format: 'png' | 'webp';
  frames?: number[];
  animated: boolean;
  testcase?: string;
  scale?: number;
  verbose: boolean;
}

interface ResolvedOptions {
  type: 'renderFrames' | 'renderAnimatedWebp' | 'generateTestPlan';
  lottieJSONPath: string;
  outputDir: string;
  width: number;
  height: number;
  format: 'png' | 'webp';
  frames: number[];
  testcase: string;
  frameRate: number;
}

const isValidURL = (string: string) => {
  try {
    new URL(string);
    return true;
  } catch {
    return false;
  }
};

const programOptionsFromCommander = (): ProgramOptions => {
  program
    .name('animax-osmesa-renderer')
    .version('0.0.1')
    .description(
      'A command-line tool for rendering Lottie JSON files using AnimaX OSMesa.'
    )
    .requiredOption(
      '-i, --input <input>',
      'Input Lottie JSON provided with URL or file path'
    )
    .requiredOption('-o, --output <output>', 'Output directory')
    .option('-w, --width <width>', 'Output width')
    .option('-h, --height <height>', 'Output height')
    .addOption(
      new Option('--format <format>', 'Output format')
        .choices(['png', 'webp'])
        .default('png')
    )
    .option(
      '--frames <frames>',
      'Frames to render (comma-separated list)',
      (value: string) => value.split(','),
      []
    )
    .option('-a, --animated', 'Render animated frames')
    .option(
      '-t, --testcase <testcase>',
      'Generate a test case with name <testCase> in the output directory. A test case is a directory containing the input Lottie JSON, a testplan JSON, and baseline images.'
    )
    .option('-s, --scale <scale>', 'Scale the output image')
    .option('-v, --verbose', 'Verbose output');

  program.parse();
  let input: string = program.opts().input;
  if (!isValidURL(input)) {
    input = path.resolve(input);
  }
  const output: string = path.resolve(program.opts().output);
  const width: number | undefined = program.opts().width;
  const height: number | undefined = program.opts().height;
  const format: 'png' | 'webp' = program.opts().format;
  const frames: number[] | undefined = program
    .opts()
    .frames.map((f: string) => parseInt(f));
  const animated: boolean = program.opts().animated;
  const testcase: string | undefined = program.opts().testcase;
  const scale: number | undefined = program.opts().scale
    ? parseFloat(program.opts().scale)
    : undefined;
  const verbose: boolean = program.opts().verbose;

  if (testcase !== undefined && testcase.length === 0) {
    throw new Error('Test case name cannot be empty');
  }

  return {
    input,
    width,
    height,
    output,
    format,
    frames,
    animated,
    testcase,
    scale,
    verbose,
  };
};

const resolveProgramOptions = async (
  options: ProgramOptions
): Promise<ResolvedOptions> => {
  const { input, output, animated, scale } = options;
  let { format, frames, width, height, testcase } = options;
  if (!isValidURL(input) && !fs.existsSync(input)) {
    throw new Error('Input is not a valid URL or file path');
  }

  let lottieJSONPath = '';

  if (isValidURL(input)) {
    const result = await prepareLottie({
      url: input,
      outputDir: output,
    });
    lottieJSONPath = result.lottie;
  }

  const lottie: Lottie = JSON.parse(
    await fs.promises.readFile(lottieJSONPath, 'utf8')
  );

  if (width === undefined || height === undefined) {
    width = lottie.w;
    height = lottie.h;
  }

  if (scale !== undefined) {
    width = Math.round(width * scale);
    height = Math.round(height * scale);
  }

  let type: ResolvedOptions['type'] = 'renderFrames';

  if (animated) {
    type = 'renderAnimatedWebp';
    frames = Array.from(
      { length: lottie.op - lottie.ip },
      (_, i) => i + lottie.ip
    );
    format = 'webp';
    testcase = '';
  } else if (testcase !== undefined) {
    type = 'generateTestPlan';
    format = 'png';
    frames = [];
  } else {
    type = 'renderFrames';
    if (frames === undefined) {
      throw new Error('Frames must be specified');
    }
    testcase = '';
  }

  return {
    type,
    lottieJSONPath,
    width,
    height,
    format,
    frames,
    testcase,
    outputDir: output,
    frameRate: lottie.fr,
  };
};

const splitTask = (options: ResolvedOptions): RenderLottieTask[] => {
  const { frames } = options;
  const tasks: RenderLottieTask[] = [];
  const frameCount = frames.length;
  for (let i = 0; i < frameCount; i += RENDERSERVER_MAX_FRAME_PER_TASK) {
    const taskFrames = frames.slice(i, i + RENDERSERVER_MAX_FRAME_PER_TASK);
    const task: RenderLottieTask = {
      ...options,
      frames: taskFrames,
    };
    tasks.push(task);
  }
  return tasks;
};

const splitAndRender = async (
  options: ResolvedOptions,
  server: RenderLottieServer
) =>
  Promise.all(
    splitTask(options).map((task) => server.renderLottieJSON(task))
  ).then((result) => result.flat().sort((lhs, rhs) => lhs.frame - rhs.frame));

const renderFrames = splitAndRender;

const renderAnimatedWebp = async (
  options: ResolvedOptions,
  server: RenderLottieServer
) =>
  splitAndRender(options, server).then(async (result) => {
    generateAnimatedWebp(
      path.join(options.outputDir, CLI_ANIMATED_WEBP_FILENAME),
      result,
      options.frameRate
    );
    await Promise.all(
      result.map(async (result) => {
        await fs.promises.unlink(result.path);
      })
    );
  });

const generatedTestPlan = async (
  options: ResolvedOptions,
  server: RenderLottieServer
) =>
  generateTestPlan({
    ...options,
    name: options.testcase,
    server,
  });

const cli = async (
  programOptions: ProgramOptions,
  server: RenderLottieServer
) => {
  const resolvedOptions = await resolveProgramOptions(programOptions);
  switch (resolvedOptions.type) {
    case 'renderFrames':
      await renderFrames(resolvedOptions, server);
      console.log(
        `Frames rendered successfully in ${resolvedOptions.outputDir}`
      );
      break;
    case 'renderAnimatedWebp':
      await renderAnimatedWebp(resolvedOptions, server);
      console.log(
        `Animated webp rendered successfully at ${path.join(
          resolvedOptions.outputDir,
          CLI_ANIMATED_WEBP_FILENAME
        )}`
      );
      break;
    case 'generateTestPlan':
      await generatedTestPlan(resolvedOptions, server);
      console.log(
        `Test plan generated successfully at ${resolvedOptions.outputDir}`
      );
      break;
  }
};

export {
  programOptionsFromCommander,
  resolveProgramOptions,
  renderFrames,
  renderAnimatedWebp,
  generatedTestPlan,
  cli,
  ProgramOptions,
  ResolvedOptions,
};
