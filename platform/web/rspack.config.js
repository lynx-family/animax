const path = require('path');
const { rspack } = require('@rspack/core');

module.exports = {
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
            type: 'commonjs',
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
