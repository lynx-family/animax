import { gatherTestPlans } from './testcase';
import path from 'path';
import { RenderLottieServer } from '../render/server';
import fs from 'fs';
import {
  TESTRUNNER_FAILED_REPORT_JSON_FILENAME,
  TESTRUNNER_GENERATED_CONTENT_DIRNAME,
} from '../constant';
import {
  generateRenderTasks,
  generateCompareTask,
  runCompareTasks,
  generatedFailedTestReports,
  imageDiffSame,
} from './testRunnerUtil';
import { program } from 'commander';

program
  .name('animax-conformance-test-runner')
  .version('0.0.1')
  .description('CLI to run animax conformance test')
  .requiredOption('-t, --testcases <testcases>', 'testcases directory')
  .requiredOption('-o, --output <output>', 'output directory');

const main = async (testcasesDir: string, outputDir: string) => {
  testcasesDir = path.resolve(testcasesDir);
  outputDir = path.resolve(outputDir);
  const server = new RenderLottieServer(true);
  try {
    await fs.promises.mkdir(outputDir, { recursive: true });
    const plans = await gatherTestPlans(testcasesDir);
    const renderTasks = generateRenderTasks(plans, outputDir);
    const renderResults = await Promise.all(
      renderTasks.map((task) => server.renderLottieJSON(task))
    );
    const compareTasks = generateCompareTask(renderTasks, renderResults, plans);
    const diffResults = await runCompareTasks(compareTasks, imageDiffSame);
    const failedTestReports = generatedFailedTestReports(
      diffResults,
      outputDir
    );

    if (failedTestReports.length !== 0) {
      console.log(JSON.stringify(failedTestReports, null, 2));
      await fs.promises.writeFile(
        path.join(outputDir, TESTRUNNER_FAILED_REPORT_JSON_FILENAME),
        JSON.stringify(failedTestReports, null, 2)
      );
      process.exit(1);
    } else {
      // Remove all generated images.
      await fs.promises.rm(
        path.join(outputDir, TESTRUNNER_GENERATED_CONTENT_DIRNAME),
        { recursive: true }
      );
      console.log('All tests passed.');
      process.exit(0);
    }
  } catch (e) {
    console.log(e);
    process.exit(1);
  } finally {
    await server.shutdown();
  }
};
program.parse();
const { testcases, output } = program.opts();
await main(testcases, output);
