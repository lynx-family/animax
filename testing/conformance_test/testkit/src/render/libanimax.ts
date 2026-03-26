import koffi from 'koffi';

const libanimax_renderer = koffi.load(LIBANIMAX_CONFORMANCE_TEST_PATH);

koffi.pointer('AnimaXRenderTask', koffi.opaque());
koffi.pointer('AnimaXRenderResult', koffi.opaque());

const animaXRenderTaskCreate = libanimax_renderer.func(
  'AnimaXRenderTask AnimaXRenderTaskCreate(const char*, int32_t, int32_t)'
);
const animaXRenderTaskRender = libanimax_renderer.func(
  'AnimaXRenderResult AnimaXRenderTaskRender(AnimaXRenderTask, int32_t)'
);
const animaXRenderTaskDestroy = libanimax_renderer.func(
  'void AnimaXRenderTaskDestroy(AnimaXRenderTask)'
);
const animaXRenderResultDestroy = libanimax_renderer.func(
  'void AnimaXRenderResultDestroy(AnimaXRenderResult)'
);
const animaXRenderResultCopyRGBABitmap = libanimax_renderer.func(
  'void AnimaXRenderResultCopyRGBABitmap(AnimaXRenderResult, _Out_ uint8_t*)'
);
const animaXRenderResultGetRGBABitmapSize = libanimax_renderer.func(
  'int32_t AnimaXRenderResultGetRGBABitmapSize(AnimaXRenderResult)'
);

declare const AnimaXOpaqueTypeTag: unique symbol;

export type AnimaXRenderTask = {
  [AnimaXOpaqueTypeTag]: 'AnimaXRenderTask';
};

export type AnimaXRenderResult = {
  [AnimaXOpaqueTypeTag]: 'AnimaXRenderResult';
};

const libanimax = {
  createRenderTask: (
    path: string,
    width: number,
    height: number
  ): AnimaXRenderTask => {
    const animaxRenderTask = animaXRenderTaskCreate(path, width, height);
    if (animaxRenderTask === null) {
      throw new Error('Failed to create AnimaXRenderTask');
    }
    return animaxRenderTask;
  },
  render: (task: AnimaXRenderTask, frame: number): Buffer => {
    const animaxRenderResult = animaXRenderTaskRender(task, frame);
    const bufferSize = animaXRenderResultGetRGBABitmapSize(animaxRenderResult);
    const buf = Buffer.allocUnsafe(bufferSize);
    animaXRenderResultCopyRGBABitmap(animaxRenderResult, buf);
    animaXRenderResultDestroy(animaxRenderResult);
    return buf;
  },
  destroyRenderTask: (task: AnimaXRenderTask) => {
    animaXRenderTaskDestroy(task);
  },
};

export default libanimax;
