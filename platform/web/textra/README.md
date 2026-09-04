# @lynx-js/animax-textra

Textra-based text layout module compiled to **WebAssembly (WASM)** for the AnimaX animation framework on the Web platform.

## Installation

Install this package only when your AnimaX animation needs Textra text layout. Keep its version aligned with `@lynx-js/animax`.

```bash
npm install @lynx-js/animax@latest \
  @lynx-js/animax-textra@latest
```

## Usage

```typescript
import { AnimaXViewElement } from '@lynx-js/animax';
import type { AnimaXFontConfig } from '@lynx-js/animax';
import { AnimaXTextraModuleUrl } from '@lynx-js/animax-textra';

const fontCdn =
  'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/fonts/';
const fontConfig: AnimaXFontConfig = {
  defaultFamily: 'Noto Sans SC',
  fonts: [
    {
      family: 'Noto Sans SC',
      url: `${fontCdn}NotoSansSC-fallback.ttf`,
    },
    {
      family: 'Noto Sans Thai',
      url: `${fontCdn}NotoSansThai-Regular.ttf`,
    },
    {
      family: 'Noto Emoji',
      url: `${fontCdn}Noto-COLRv1.ttf`,
    },
  ],
};

const [fontsLoaded, textraLoaded] = await Promise.all([
  AnimaXViewElement.configureFonts(fontConfig),
  AnimaXViewElement.loadTextraModule(AnimaXTextraModuleUrl),
]);

if (!fontsLoaded || !textraLoaded) {
  throw new Error('Failed to load AnimaX Textra runtime');
}
```

Load the module once before rendering animations that need Textra text layout. For precise multi-language fallback on Web, configure fonts from the core package with `AnimaXViewElement.configureFonts(...)` before mounting `<animax-view>`.

Font descriptors are intentionally minimal:

```typescript
type AnimaXFontDescriptor = {
  family: string;
  url: string;
};
```

Fallback order follows the order in `fonts`. `defaultFamily` marks the configured default font. `aliases`, `roles`, `fallbackPriority`, and `fallbackOrder` are not supported by the Web public API.

## Build

Textra's wasm-consumable native source target is declared by
`third_party/textlayout/textra/platform/wasm:lynxtextra_static`; AnimaX wires it
into `animax_textra.wasm` from `animax/src/BUILD.gn` and `animax/src/animax.gni`.
This package only owns npm packaging and wasm artifact output.

```bash
npm install
npm run build
```

The build script runs:

```bash
python3 ../../../tools/build_wasm.py -t Release --package textra
npx tsc --project tsconfig.json
```

## Output

```text
@lynx-js/animax-textra/
└── out/
    ├── index.js
    ├── index.d.ts
    └── lib/
        └── animax-textra.wasm
```

The runtime side module name remains `animax_textra.wasm`; `@lynx-js/animax` maps that name to this package URL when `loadTextraModule()` is called.

## License

Apache-2.0
