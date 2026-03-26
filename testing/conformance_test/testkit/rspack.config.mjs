import path from 'path';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';
import { TsCheckerRspackPlugin } from 'ts-checker-rspack-plugin';
import rspack from '@rspack/core';
import nodeExternals from 'webpack-node-externals';
import os from 'os';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const platform = os.platform();

let LIBANIMAX_CONFORMANCE_TEST_PATH = '';
if (platform === 'darwin') {
  LIBANIMAX_CONFORMANCE_TEST_PATH = path.resolve(
    __dirname,
    'dist/libanimax_conformance_test.dylib'
  );
} else if (platform === 'linux') {
  LIBANIMAX_CONFORMANCE_TEST_PATH = path.resolve(
    __dirname,
    'dist/libanimax_conformance_test.so'
  );
} else {
  console.error('Unsupported platform:', platform);
  process.exit(1);
}

const LIBANIMAX_CONFORMANCE_TEST_BUILD_SCRIPT = path.resolve(
  __dirname,
  'scripts/build_libanimax_conformance_test.sh'
);
const OUTPUT_DIR = path.resolve(__dirname, 'dist');
const RENDER_API_OUTPUT_FILENAME = 'render-api.cjs';
const RENDER_API_OUTPUT_PATH = path.resolve(
  OUTPUT_DIR,
  RENDER_API_OUTPUT_FILENAME
);

const compileAndCopyLibAnimaXConformanceTest = () => {
  execSync(`sh ${LIBANIMAX_CONFORMANCE_TEST_BUILD_SCRIPT} ${OUTPUT_DIR}`, {
    stdio: 'inherit',
  });
};

const commonConfig = {
  devtool: 'source-map',
  mode: 'development',
  target: 'node',
  watchOptions: {
    ignored: /node_modules/,
    aggregateTimeout: 300,
    poll: undefined,
  },
  module: {
    rules: [
      {
        test: /\.ts$/,
        exclude: [/node_modules/],
        loader: 'builtin:swc-loader',
        options: {
          jsc: {
            parser: {
              syntax: 'typescript',
            },
          },
        },
        type: 'javascript/auto',
      },
    ],
  },
  externals: [nodeExternals()],
  resolve: {
    // Enable using import a from './a' instead of './a.ts'
    extensions: ['.tsx', '.ts', '.js'],
  },
  plugins: [
    {
      apply: (compiler) => {
        compiler.hooks.afterEmit.tapAsync(
          'CopyAndCopyLibAnimaXConformanceTestPlugin',
          (compilation, callback) => {
            try {
              compileAndCopyLibAnimaXConformanceTest();
            } catch (err) {
              compilation.errors.push(
                new Error(
                  `CopyAndCopyLibAnimaXConformanceTestPlugin: ${err.message}`
                )
              );
            }
            callback();
          }
        );
      },
    },
    new rspack.DefinePlugin({
      LIBANIMAX_CONFORMANCE_TEST_PATH: JSON.stringify(
        LIBANIMAX_CONFORMANCE_TEST_PATH
      ),
      RENDER_API_OUTPUT_PATH: JSON.stringify(RENDER_API_OUTPUT_PATH),
    }),
    new TsCheckerRspackPlugin(),
  ],
};

const config = [
  {
    ...commonConfig,
    entry: {
      'render-cli': './src/render/cli.ts',
      'render-server': './src/render/server.ts',
      'test-runner': './src/testcase/testRunner.ts',
    },
    output: {
      filename: '[name].cjs',
      path: OUTPUT_DIR,
    },
  },
  {
    ...commonConfig,
    entry: {
      'render-api': './src/render/render.ts',
    },
    output: {
      // render-api must be emitted as commonjs since it will be dynamic-required
      filename: RENDER_API_OUTPUT_FILENAME,
      libraryTarget: 'commonjs2',
      path: OUTPUT_DIR,
    },
  },
];

export default config;
