import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { rspack } from '@rspack/core';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

export default {
    entry: './src/index.ts',
    resolve: {
        extensions: ['.tsx', '.ts', '.js'],
    },
    mode: 'development',
    experiments: {
        outputModule: true,
    },
    output: {
        filename: 'index.js',
        path: path.resolve(__dirname, 'out'),
        library: {
            type: 'module',
        },
    },
    module: {
        rules: [
            {
                test: /\.ts$/,
                use: {
                    loader: 'builtin:swc-loader',
                    options: {
                        jsc: {
                            parser: {
                                syntax: 'typescript',
                            },
                        },
                    },
                },
                exclude: /node_modules/,
            },
            {
                test: /\.tsx$/,
                use: {
                    loader: 'builtin:swc-loader',
                    options: {
                        jsc: {
                            parser: {
                                syntax: 'typescript',
                                tsx: true,
                            },
                        },
                    },
                },
                exclude: /node_modules/,
            },
        ],
    },
    plugins: [
        new rspack.CopyRspackPlugin({
            patterns: [
                {
                    from: path.resolve(__dirname, 'public'),
                    to: path.resolve(__dirname, 'out/public'),
                },
            ],
        }),
    ],
};
