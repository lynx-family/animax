import { Worker } from 'jest-worker';
import type { RenderLottieJSON, RenderLottieTask } from './render';

declare module 'jest-worker' {
  interface Worker {
    renderLottieJSON: RenderLottieJSON;
  }
}

class RenderLottieServer {
  private worker: Worker;

  constructor(verbose: boolean) {
    this.worker = new Worker(RENDER_API_OUTPUT_PATH);
    if (verbose) {
      this.worker.getStdout().on('data', (data) => {
        console.log(data.toString());
      });
      this.worker.getStderr().on('data', (data) => {
        console.error(data.toString());
      });
    }
  }

  async renderLottieJSON(renderTask: RenderLottieTask) {
    return this.worker.renderLottieJSON(renderTask);
  }

  async shutdown() {
    const result = await this.worker.end();
    const { forceExited } = result;
    if (forceExited) {
      console.error('RenderLottieServer subprocess failed to exit gracefully.');
    }
  }
}

export { RenderLottieServer };
