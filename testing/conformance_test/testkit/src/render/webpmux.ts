import { execSync } from 'child_process';
import type { RenderLottieResult } from './render';

const isWebpmuxInstalled = () => {
  try {
    // Run the `webpmux -version` command to check if it's installed
    execSync('webpmux -version', { stdio: 'ignore' });
    return true;
  } catch {
    console.error('Error: webpmux is not installed.');
    return false;
  }
};

const generateWebpmuxCommand = (
  renderLottieResults: RenderLottieResult[],
  frameRate: number,
  outputWebpFilePath: string
): string => {
  // List all files in the directory and filter for .webp files
  const webpFiles = renderLottieResults.map((result) => result.path);

  if (webpFiles.length === 0) {
    throw new Error('Error: No .webp files found in the current directory.');
  }

  // Sort files (natural order)
  webpFiles.sort((a, b) => a.localeCompare(b, undefined, { numeric: true }));

  // Generate the frames part of the command
  const frames = webpFiles
    .map((file) => `-frame ${file} +${Math.round(1000 / frameRate)}+0+0+1-b`)
    .join(' ');

  // Construct the final webpmux command
  const command = `webpmux -loop 0 -bgcolor 255,255,255,255 ${frames} -o ${outputWebpFilePath}`;
  return command;
};

const generateAnimatedWebp = (
  output: string,
  renderLottieResults: RenderLottieResult[],
  fps: number
) => {
  if (!isWebpmuxInstalled()) {
    throw new Error('Error: webpmux is not installed.');
  }
  const animatedWebpPath = output;
  const frameRate = fps ?? 30;
  const command = generateWebpmuxCommand(
    renderLottieResults,
    frameRate,
    animatedWebpPath
  );
  console.log('Generating animated webp...');
  execSync(command, { stdio: 'inherit' });
  console.log(`Animated webp generated: ${animatedWebpPath}`);
};

export { generateAnimatedWebp };
