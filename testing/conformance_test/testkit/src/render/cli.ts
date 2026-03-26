import { cli, programOptionsFromCommander } from './cliUtil';
import { RenderLottieServer } from './server';

const main = async () => {
  const programOptions = programOptionsFromCommander();
  const server = new RenderLottieServer(false);
  await cli(programOptions, server);
  process.exit(0);
};

await main();
