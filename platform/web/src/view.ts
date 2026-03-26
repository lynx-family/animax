import animax_wasm from '../public/build/animax_wasm.js';
import { AnimaXResourceLoader } from './resource';
import { AnimaXWebGPUContext } from './gpu';
import { Logger } from './log';
import {
  AnimaXViewProps,
  AnimaXViewController,
  AnimaXValueParam,
  AnimaXPropertyCallback,
  AnimaXPoint,
  AnimaXLayerPropertyType,
  AnimaXResourcePropertyType,
} from './view-types';

const SafeHTMLElement =
  typeof HTMLElement !== 'undefined'
    ? HTMLElement
    : (class {} as typeof HTMLElement);

class AnimaXViewElement
  extends SafeHTMLElement
  implements AnimaXViewProps, AnimaXViewController {
  // --- DOM attributes (props) ---
  public backend: 'webgl' | 'webgpu' = 'webgl';
  public src: string = '';
  public 'src-format': string = '';
  public 'src-polyfill': Record<string, string> | string = '';
  public json: string = '';
  public speed: number = 1.0;
  public autoplay: boolean = true;
  public 'start-frame': number = 0;
  public 'end-frame': number = -1;
  public 'auto-reverse': boolean = false;
  public progress: number = 0;
  public loop: boolean = false;
  public 'loop-count': number = 1;
  public objectfit: 'contain' | 'cover' | 'center' | 'fill' | 'scale-down' =
    'contain';
  public 'object-position':
    | 'center'
    | 'left'
    | 'right'
    | 'top'
    | 'bottom'
    | 'top-left'
    | 'top-right'
    | 'bottom-left'
    | 'bottom-right' = 'center';
  public 'fps-event-interval': number = 0;
  public 'max-frame-rate': number = 60;
  public 'dynamic-resource': boolean = false;
  public 'keeplastframe': boolean = true;
  // --- Static members ---
  private static _instanceCount = 0;
  private static _loadedFontHash: string | null = null;
  private static _moduleInstance: any = null;
  private static _webgpuContext: AnimaXWebGPUContext | null = null;
  // --- Internal members ---
  private _canvas: HTMLCanvasElement | null = null;
  private _module: any = null;
  private _wasm: any = null;
  private _player: any = null;
  private _resourceLoader: AnimaXResourceLoader | null = null;
  private _visibilityFlag: number = 0;
  private _intersectionObserver?: IntersectionObserver;
  private _mutationObserver?: MutationObserver;
  private _resizeObserver?: ResizeObserver;
  private _pendingDimensionUpdate = false;
  static get observedAttributes() {
    return [
      'backend',
      'src',
      'src-format',
      'src-polyfill',
      'width',
      'height',
      'json',
      'speed',
      'autoplay',
      'start-frame',
      'end-frame',
      'auto-reverse',
      'progress',
      'loop',
      'loop-count',
      'objectfit',
      'object-position',
      'fps-event-interval',
      'max-frame-rate',
      'dynamic-resource',
      'keeplastframe',
    ];
  }
  // --- Playback control ---
  public play(): void {
    this._player?.play();
  }
  public pause(): void {
    this._player?.pause();
  }
  public resume(): void {
    this._player?.resume();
  }
  public stop(): void {
    this._player?.stop();
  }
  public reload(): void {
    this._player?.reload();
  }
  public seek(frame: number): void {
    this._player?.seek(frame);
  }
  public playSegment(startFrame: number, endFrame: number): void {
    this._player?.playSegment(startFrame, endFrame);
  }
  public isAnimating(): boolean {
    return this._player?.isAnimating() ?? false;
  }
  public getDuration(): number {
    return this._player?.getDurationMs() ?? 0;
  }
  public getCurrentFrame(): number {
    return this._player?.getCurrentFrame() ?? 0;
  }
  // --- Playback configuration ---
  public setLoop(loop: boolean): void {
    this._player?.setLoop(loop);
  }
  public setLoopCount(loopCount: number): void {
    this._player?.setLoopCount(loopCount);
  }
  public setProgress(progress: number): void {
    this._player?.setProgress(progress);
  }
  public setAutoPlay(autoplay: boolean): void {
    this._player?.setAutoPlay(autoplay);
  }
  public setSpeed(speed: number): void {
    this._player?.setSpeed(speed);
  }
  public setStartFrame(startFrame: number): void {
    this._player?.setStartFrame(startFrame);
  }
  public setEndFrame(endFrame: number): void {
    this._player?.setEndFrame(endFrame);
  }
  public setAutoReverse(autoReverse: boolean): void {
    this._player?.setAutoReverse(autoReverse);
  }
  public setKeepLastFrame(keepLastFrame: boolean): void {
    this._player?.setKeepLastFrame(keepLastFrame);
  }
  public setObjectFit(objectFit: string): void {
    let enumValue: number;
    const normalized = objectFit.toLowerCase().trim();
    switch (normalized) {
      case 'center':
        enumValue = 0;
        break;
      case 'cover':
        enumValue = 1;
        break;
      case 'fill':
        enumValue = 3;
        break;
      case 'scale-down':
        enumValue = 4;
        break;
      case 'contain':
      default:
        enumValue = 2;
        break;
    }
    this._player?.setObjectFit(enumValue);
  }
  public setObjectPosition(objectPosition: string): void {
    let enumValue: number;
    const normalized = objectPosition.toLowerCase().trim();
    switch (normalized) {
      case 'left':
        enumValue = 1;
        break;
      case 'right':
        enumValue = 2;
        break;
      case 'top':
        enumValue = 3;
        break;
      case 'bottom':
        enumValue = 4;
        break;
      case 'top-left':
        enumValue = 5;
        break;
      case 'top-right':
        enumValue = 6;
        break;
      case 'bottom-left':
        enumValue = 7;
        break;
      case 'bottom-right':
        enumValue = 8;
        break;
      case 'center':
      default:
        enumValue = 0;
        break;
    }
    this._player?.setObjectPosition(enumValue);
  }
  public setFpsEventInterval(interval: number): void {
    this._player?.setFpsEventInterval(interval);
  }
  public setMaxFrameRate(maxFrameRate: number): void {
    this._player?.setMaxFrameRate(maxFrameRate);
  }
  public setDynamicResource(dynamic: boolean): void {
    this._player?.setDynamicResource(dynamic);
  }
  // --- Animation source & resources ---
  public setJson(json: string): void {
    this._player?.setJson(json);
  }
  public setImageFolder(imageFolder: string): void {
    this._player?.setImageFolder(imageFolder);
  }
  public setSrc(src: string): void {
    this._player?.setSrc(src);
  }
  public setSrcPolyfill(polyfill: Record<string, string> | string): void {
    if (typeof polyfill === 'string') {
      try {
        this._player?.setSrcPolyfill(JSON.parse(polyfill));
      } catch {
        this._player?.setSrcPolyfill({});
      }
    } else {
      this._player?.setSrcPolyfill(polyfill || {});
    }
  }
  // --- Interaction and Events ---
  public subscribeUpdateEvent(frame: number): void {
    this._player?.subscribeUpdateEvent(frame);
  }
  public unsubscribeUpdateEvent(frame: number): void {
    this._player?.unsubscribeUpdateEvent(frame);
  }
  public subscribeUpdateEvents(frames: number[]): void {
    this._player?.subscribeUpdateEvents(frames, true);
  }
  public unsubscribeUpdateEvents(frames: number[]): void {
    this._player?.unsubscribeUpdateEvents(frames, false);
  }
  // --- Property update ---
  public setResourceProperty(
    resource_type: AnimaXResourcePropertyType,
    resource_id: string,
    value: AnimaXValueParam,
    callback?: AnimaXPropertyCallback
  ): void {
    const propertyCallback = callback ?? (() => {});
    this._wasm?.setResourceProperty(
      resource_type,
      resource_id,
      value,
      propertyCallback
    );
  }
  public updateLayerProperty(
    layer_type: AnimaXLayerPropertyType,
    layer_name: string,
    value: AnimaXValueParam,
    callback?: AnimaXPropertyCallback
  ): void {
    const propertyCallback = callback ?? (() => {});
    this.autoAdaptValueParam(layer_type, value);
    this._wasm?.updateLayerProperty(
      layer_type,
      layer_name,
      value,
      propertyCallback
    );
  }
  public updateTextSizeByLayerName(
    layerName: string,
    textSize: number,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): void {
    this.updateLayerProperty(
      AnimaXLayerPropertyType.TextSize,
      layerName,
      createAnimaXValueParam(textSize, targetFrame),
      callback
    );
  }
  public updateTextColorByLayerName(
    layerName: string,
    textColor: string,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): boolean {
    const hexTextColor = this.formatColorString(textColor);
    if (hexTextColor === null) {
      return false;
    }
    this.updateLayerProperty(
      AnimaXLayerPropertyType.TextColor,
      layerName,
      createAnimaXValueParam(hexTextColor, targetFrame),
      callback
    );
    return true;
  }
  public updateTextByLayerName(
    layerName: string,
    newText: string,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): void {
    this.updateLayerProperty(
      AnimaXLayerPropertyType.TextValue,
      layerName,
      createAnimaXValueParam(newText, targetFrame),
      callback
    );
  }
  public updateImageById(imageId: string, newImageUrl: string): void {
    this.setResourceProperty(
      AnimaXResourcePropertyType.ImageDirName,
      imageId,
      createAnimaXValueParam('')
    );
    this.setResourceProperty(
      AnimaXResourcePropertyType.ImageFileName,
      imageId,
      createAnimaXValueParam(newImageUrl)
    );
  }
  public updateVideoById(videoId: string, newVideoUrl: string): void {
    this.setResourceProperty(
      AnimaXResourcePropertyType.VideoDirName,
      videoId,
      createAnimaXValueParam('')
    );
    this.setResourceProperty(
      AnimaXResourcePropertyType.VideoFileName,
      videoId,
      createAnimaXValueParam(newVideoUrl)
    );
  }
  public updateFontByName(fontName: string, newFontPath: string): void {
    this.setResourceProperty(
      AnimaXResourcePropertyType.FontPath,
      fontName,
      createAnimaXValueParam(newFontPath)
    );
  }
  /**
   * Load default font for all instances (singleton)
   */
  public static async loadDefaultFont(url: string): Promise<boolean> {
    const urlHash = this.hashString(url);
    if (this._loadedFontHash === urlHash) return true;
    const response = await fetch(url);
    if (!response.ok) return false;
    const arrayBuffer = await response.arrayBuffer();
    if (arrayBuffer.byteLength <= 0) return false;
    const animax = await this._getAnimaxModule();
    const result = await animax.AnimaXWasm.setDefaultTypefaceWithData(
      new Uint8Array(arrayBuffer)
    );
    if (result) this._loadedFontHash = urlHash;
    return result;
  }

  // --- Lifecycle ---
  constructor() {
    super();
  }

  connectedCallback() {
    window.addEventListener('beforeunload', this.clearPlayerAndListeners);
    this.setupVisibilityListeners();
    this.initPlayer();
  }
  disconnectedCallback() {
    window.removeEventListener('beforeunload', this.clearPlayerAndListeners);
    Logger.info('clearPlayerAndListeners');
    this.clearPlayerAndListeners();
  }
  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (oldValue === newValue) return;
    if (name === 'backend') {
      if (this._player) {
        Logger.info('backend changed, destroy player and create new player');
        this.destroyPlayer();
        this._canvas?.remove();
        this._canvas = null;
        this.initPlayer();
      }
    } else if (name === 'width' || name === 'height') {
      this.scheduleDimensionChange();
    } else {
      this.handleAttribute(name, newValue);
    }
  }
  // --- Player Initialization ---
  private async initPlayer() {
    Logger.info('initPlayer');
    this._module = await AnimaXViewElement._getAnimaxModule();
    if (this._resourceLoader) {
      this._resourceLoader.destroy();
    }
    this._resourceLoader = new AnimaXResourceLoader(
      this._module,
      await this.getWebGPUContext()
    );
    this._wasm = new this._module.AnimaXWasm();
    this._player = this._wasm.getPlayer();
    this._wasm.setResourceLoaderImpl(
      this._resourceLoader.loadUrl.bind(this._resourceLoader)
    );
    this._wasm.setEventCallback((eventName: string, params: any) => {
      Logger.info(
        `dispatch event ${eventName} with params ${JSON.stringify(params)}`
      );
      this.dispatchEvent(
        new CustomEvent(eventName, {
          detail: params,
          bubbles: false,
        })
      );
    });

    this.ensureInnerCanvas();
    this.handleDimensionChange(true);
    this.bindCanvasTapEvent();
    for (let i = 0; i < this.attributes.length; i++) {
      const attr = this.attributes[i];
      this.handleAttribute(attr.name, attr.value);
    }
  }

  private destroyPlayer() {
    if (this._player) {
      this.unbindCanvasTapEvent();
      this._player.stop();
      this._player.delete();
      this._player = null;
    }
    if (this._resourceLoader) {
      this._resourceLoader.destroy();
      this._resourceLoader = null;
    }
    if (this._wasm) {
      this._wasm.delete();
      this._wasm = null;
    }
  }
  // --- Canvas management ---
  private ensureInnerCanvas() {
    if (this._canvas) return;
    AnimaXViewElement._instanceCount++;
    this._canvas = document.createElement('canvas');
    this._canvas.id = `animax-canvas-${Date.now()}-${Math.floor(
      Math.random() * 10000
    )}-${AnimaXViewElement._instanceCount}`;
    Logger.info(`create canvas ${this._canvas.id}`);
    this._canvas.style.display = 'block';
    this._canvas.style.width = '100%';
    this._canvas.style.height = '100%';
    this.appendChild(this._canvas);
  }
  private scheduleDimensionChange() {
    if (this._pendingDimensionUpdate) return;
    this._pendingDimensionUpdate = true;
    Promise.resolve().then(() => {
      this._pendingDimensionUpdate = false;
      this.handleDimensionChange();
    });
  }
  private async getWebGPUContext() {
    const backend = this.getAttribute('backend');
    let webgpuContext: any = null;
    if (backend === 'webgpu') {
      webgpuContext = await AnimaXViewElement.getWebGPUContext();
    }
    return webgpuContext;
  }

  private async handleDimensionChange(forceUpdate: boolean = false) {
    if (!this._canvas) {
      Logger.info('handleDimensionChange failed, canvas not ready');
      return;
    }
    const pixelRatio = this.getPixelRatio();
    const width =
      Number(this.getAttribute('width')) ||
      this.offsetWidth * pixelRatio ||
      300 * pixelRatio;
    const height =
      Number(this.getAttribute('height')) ||
      this.offsetHeight * pixelRatio ||
      150 * pixelRatio;
    if (
      forceUpdate ||
      this._canvas.width !== width ||
      this._canvas.height !== height
    ) {
      this._canvas.width = width;
      this._canvas.height = height;
      const backend = this.getAttribute('backend');
      let wasmContext: any = null;
      let glContextHandle: number = 0;

      if (backend === 'webgpu') {
        wasmContext = (await AnimaXViewElement.getWebGPUContext()).wasmContext;
      } else {
        let gl:
          | WebGL2RenderingContext
          | WebGLRenderingContext
          | null = this._canvas.getContext('webgl2', {
          stencil: true,
          antialias: false,
          powerPreference: 'high-performance',
        });
        // Fallback to WebGL 1 if WebGL 2 is not available
        if (!gl) {
          gl = this._canvas.getContext('webgl', {
            stencil: true,
            antialias: false,
            powerPreference: 'high-performance',
          });
        }
        if (gl && this._module.GL) {
          glContextHandle = this._module.GL.registerContext(gl, {
            majorVersion: 2,
            minorVersion: 0,
          });
        } else {
          Logger.info(
            `Failed to create WebGL context in JS, gl: ${gl}, GL: ${this._module.GL}`
          );
        }
      }
      this._wasm.setSurfaceWithCanvas(
        '#' + this._canvas.id,
        width,
        height,
        wasmContext,
        glContextHandle
      );
    }
  }
  // --- Attribute Handling ---
  private handleAttribute(name: string, value: string) {
    if (!this._player) return;
    switch (name) {
      case 'src':
      case 'src-format':
        this._player.setSrc(value);
        break;
      case 'src-polyfill':
        if (typeof value === 'string') {
          try {
            this._player.setSrcPolyfill(JSON.parse(value));
          } catch {
            this._player.setSrcPolyfill({});
          }
        } else {
          this._player.setSrcPolyfill(value || {});
        }
        break;
      case 'json':
        this._player.setJson(value);
        break;
      case 'backend':
      case 'width':
      case 'height':
        // handled in attributeChangedCallback
        break;
      case 'loop':
        this._player.setLoop(value === 'true' || value === '');
        break;
      case 'autoplay':
        this._player.setAutoPlay(value === 'true' || value === '');
        break;
      case 'speed':
        this._player.setSpeed(parseFloat(value) || 1);
        break;
      case 'start-frame':
        this._player.setStartFrame(parseInt(value) || 0);
        break;
      case 'end-frame':
        this._player.setEndFrame(parseInt(value) || -1);
        break;
      case 'auto-reverse':
        this._player.setAutoReverse(value === 'true' || value === '');
        break;
      case 'progress':
        this._player.setProgress(parseFloat(value) || 0);
        break;
      case 'loop-count':
        this._player.setLoopCount(parseInt(value) || 1);
        break;
      case 'objectfit':
        this.setObjectFit(value);
        break;
      case 'object-position':
        this.setObjectPosition(value);
        break;
      case 'fps-event-interval':
        this._player.setFpsEventInterval(parseInt(value) || 0);
        break;
      case 'max-frame-rate':
        this._player.setMaxFrameRate(parseInt(value) || 60);
        break;
      case 'dynamic-resource':
        this._player.setDynamicResource(value === 'true' || value === '');
        break;
      case 'keeplastframe':
        this._player.setKeepLastFrame(value === 'true' || value === '');
        break;
      default:
        break;
    }
  }
  // --- Canvas tap event binding ---
  private bindCanvasTapEvent(): void {
    const handleTap = (event: MouseEvent) => {
      const rect = this._canvas!.getBoundingClientRect();
      const scale = this.getPixelRatio();
      const x = (event.clientX - rect.left) * scale;
      const y = (event.clientY - rect.top) * scale;
      this._player.onTap(x, y);
    };
    this._canvas.addEventListener('click', handleTap);
    (this._canvas as any)._handleTap = handleTap;
  }
  private unbindCanvasTapEvent(): void {
    const handleTap = (this._canvas as any)._handleTap;
    if (handleTap) {
      this._canvas.removeEventListener('click', handleTap);
      delete (this._canvas as any)._handleTap;
    }
  }
  // --- Visibility listeners ---
  private clearPlayerAndListeners = () => {
    this.destroyPlayer();
    this.clearVisibilityListeners();
  };
  private setupVisibilityListeners() {
    // Listen for page visibility changes
    document.addEventListener(
      'visibilitychange',
      this.updateVisibilityStateHandler
    );
    // Listen for intersection (element enters or leaves the viewport)
    this._intersectionObserver = new IntersectionObserver((entries) => {
      // Trigger only when intersection state changes
      if (entries[0].isIntersecting !== undefined) {
        this.updateVisibilityStateHandler();
      }
    });
    this._intersectionObserver.observe(this);
    // Listen for element size changes
    this._resizeObserver = new ResizeObserver(() => {
      this.updateVisibilityStateHandler();
      this.handleDimensionChange();
    });
    this._resizeObserver.observe(this);
    // Listen for element being added to or removed from its parent
    if (this.parentNode) {
      this._mutationObserver = new MutationObserver((mutations) => {
        for (const mutation of mutations) {
          if (
            Array.from(mutation.addedNodes).includes(this) ||
            Array.from(mutation.removedNodes).includes(this)
          ) {
            this.updateVisibilityStateHandler();
            break;
          }
        }
      });
      this._mutationObserver.observe(this.parentNode, {
        childList: true,
      });
    }
  }
  private clearVisibilityListeners() {
    // Remove page visibility change listener
    document.removeEventListener(
      'visibilitychange',
      this.updateVisibilityStateHandler
    );
    // Disconnect intersection observer
    this._intersectionObserver?.disconnect();
    // Disconnect resize observer
    this._resizeObserver?.disconnect();
    // Disconnect mutation observer
    this._mutationObserver?.disconnect();
  }

  private updateVisibilityStateHandler = () => {
    if (!this._player) {
      return;
    }

    const VisibilityState = {
      Background: 1,
      Visible: 2,
      Opacity: 4,
      Size: 8,
      Attach: 16,
    };
    let state = 0;
    if (document.visibilityState !== 'visible') {
      state |= VisibilityState.Background;
    }
    if (
      (this as any).checkVisibility ? (this as any).checkVisibility() : true
    ) {
      // Visible, do nothing (this is a positive flag, not a negative one)
    } else {
      state |= VisibilityState.Visible;
    }
    if (parseFloat(getComputedStyle(this).opacity) < 0.01) {
      state |= VisibilityState.Opacity;
    }
    if (this.offsetWidth === 0 || this.offsetHeight === 0) {
      state |= VisibilityState.Size;
    }
    if (!this.isConnected) {
      state |= VisibilityState.Attach;
    }
    if (this._visibilityFlag === state) {
      return;
    }
    this._visibilityFlag = state;
    Logger.info(`update visibility states to ${state}`);
    this._wasm?.updateVisibilityStates(state);
  };

  private static async _getAnimaxModule(): Promise<any> {
    if (!AnimaXViewElement._moduleInstance) {
      AnimaXViewElement._moduleInstance = await animax_wasm().catch(
        (error: any) => {
          Logger.error('Failed to load animax-wasm binding:', error);
          throw error;
        }
      );
    }
    return AnimaXViewElement._moduleInstance;
  }

  private static async getWebGPUContext() {
    if (!AnimaXViewElement._webgpuContext) {
      const animax = await this._getAnimaxModule();
      AnimaXViewElement._webgpuContext = await AnimaXWebGPUContext.create(
        animax
      );
    }
    return AnimaXViewElement._webgpuContext;
  }

  private autoScaleLayerProperties = new Set<AnimaXLayerPropertyType>([
    AnimaXLayerPropertyType.TransformAnchor,
    AnimaXLayerPropertyType.TransformPosition,
    AnimaXLayerPropertyType.TextSize,
  ]);

  private getPixelRatio(): number {
    const pixelRatio =
      typeof window !== 'undefined' ? window.devicePixelRatio : 1;
    return Number.isFinite(pixelRatio) ? pixelRatio : 1;
  }

  private autoAdaptValueParam(
    layerType: AnimaXLayerPropertyType,
    param: AnimaXValueParam
  ): void {
    const density = this.getPixelRatio();
    if (
      !this.autoScaleLayerProperties.has(layerType) ||
      density === 1 ||
      !param
    ) {
      return;
    }
    // Scale point properties if both are valid numbers
    if (Number.isFinite(param.pointX) && Number.isFinite(param.pointY)) {
      param.pointX! *= density;
      param.pointY! *= density;
      return;
    }
    // Scale double value property if valid
    if (Number.isFinite(param.doubleValue)) {
      param.doubleValue! *= density;
    }
  }

  private formatColorString(color: string): string | null {
    if (
      typeof color !== 'string' ||
      // eslint-disable-next-line regexp/no-unused-capturing-group
      !/^(#|0x)([0-9a-fA-F]{2}){3,4}$/.test(color)
    ) {
      return null;
    }
    // Remove the leading "#" or "0x"
    const hexColor = color.slice(color.startsWith('#') ? 1 : 2);
    return hexColor;
  }

  private static hashString(str: string): string {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      hash = (hash << 5) - hash + str.charCodeAt(i);
      hash |= 0;
    }
    return hash.toString();
  }

  private truncate(str: string, maxLen: number = 30): string {
    if (str.length <= maxLen) return str;
    return str.slice(0, maxLen) + '...';
  }
}

export function createAnimaXValueParam(
  value: string | number | boolean | AnimaXPoint,
  targetFrame?: number
): AnimaXValueParam {
  const frameIndex = targetFrame ?? -1;
  if (typeof value === 'object' && 'x' in value && 'y' in value) {
    return {
      pointX: value.x,
      pointY: value.y,
      frameIndex,
    };
  } else if (typeof value === 'string') {
    return {
      stringValue: value,
      frameIndex,
    };
  } else if (typeof value === 'number') {
    return {
      doubleValue: value,
      frameIndex,
    };
  } else if (typeof value === 'boolean') {
    return {
      boolValue: value,
      frameIndex,
    };
  } else {
    // No matching type value, return a ValueParam with only frameIndex.
    return { frameIndex };
  }
}

// Register custom element
if (typeof window !== 'undefined' && !customElements.get('animax-view')) {
  customElements.define('animax-view', AnimaXViewElement);
}
export { AnimaXViewElement };
