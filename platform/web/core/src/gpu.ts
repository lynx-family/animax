/// <reference types="@webgpu/types" />
import { Logger } from './log.js';

export class AnimaXWebGPUContext {
  readonly module: any;
  readonly device: GPUDevice;
  readonly queue: GPUQueue;

  private __wasmContext: any;
  // Prevent double-destroy
  private __destroyed = false;

  /**
   * @internal
   * @hideconstructor
   */
  constructor(
    module: any,
    device: GPUDevice,
    queue: GPUQueue,
    wasmContext: any
  ) {
    this.module = module;
    this.device = device;
    this.queue = queue;
    this.__wasmContext = wasmContext;
  }

  /**
   * @internal
   */
  get wasmContext() {
    return this.__wasmContext;
  }

  /**
   * Destroy the context and all textures created by this context.
   * Safe to call multiple times.
   */
  destroy(): void {
    if (this.__destroyed) {
      return;
    }
    this.__destroyed = true;
    // Then destroy wasm context
    try {
      this.__wasmContext?.delete?.();
    } finally {
      this.__wasmContext = null;
    }
  }

  /**
   * Create a GPUTexture from an ImageBitmap and register it for later destruction.
   * Returns a wasm-side handle for the imported JS texture.
   */
  createTexture(bitmap: ImageBitmap): { handle: number; texture: GPUTexture } {
    if (this.__destroyed) {
      throw new Error('AnimaXWebGPUContext has been destroyed');
    }
    const texture = this.device.createTexture({
      size: [bitmap.width, bitmap.height],
      format: 'rgba8unorm',
      usage:
        GPUTextureUsage.COPY_DST |
        GPUTextureUsage.TEXTURE_BINDING |
        GPUTextureUsage.RENDER_ATTACHMENT,
    });
    this.queue.copyExternalImageToTexture(
      { source: bitmap },
      { texture, premultipliedAlpha: true },
      [bitmap.width, bitmap.height]
    );
    const handle: number = this.module.WebGPU.importJsTexture(texture);
    return { handle, texture };
  }

  static async create(module: any): Promise<AnimaXWebGPUContext> {
    if (!navigator.gpu) {
      Logger.error('WebGPU not supported on this environment');
      throw new Error('WebGPU not supported on this environment');
    }

    const adapter = await navigator.gpu.requestAdapter();
    if (!adapter) {
      Logger.error('Failed to request WebGPU adapter');
      throw new Error('Failed to request WebGPU adapter');
    }

    const device = await adapter.requestDevice({
      label: 'Animax WebGPU Device',
    });

    const queue = device.queue;
    queue.label = 'Animax WebGPU Queue';

    const devicePtr = module.WebGPU.importJsDevice(device);
    const queuePtr = module.WebGPU.importJsQueue(queue);

    const wasmContext = module.AnimaXWebGPUContext.create(devicePtr, queuePtr);

    if (!wasmContext) {
      Logger.error('Failed to create AnimaXWebGPUContext');
      throw new Error('Failed to create AnimaXWebGPUContext');
    }

    Logger.info('AnimaXWebGPUContext created successfully');
    return new AnimaXWebGPUContext(module, device, queue, wasmContext);
  }
}
