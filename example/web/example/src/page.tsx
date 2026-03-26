import React, { useState, useRef, useEffect } from 'react';
import Slider from 'rc-slider';
import 'rc-slider/assets/index.css';
import { LottieView } from './lottie_view';
import { urls } from './urls';
import { AnimaXViewElement, AnimaXViewProps } from '@byted-lynx/animax';
import { AnimaPageUrl } from './urls';
// TypeScript declaration for custom element
declare global {
  namespace JSX {
    interface IntrinsicElements {
      'animax-view': React.DetailedHTMLProps<
        React.HTMLAttributes<AnimaXViewElement>,
        AnimaXViewElement
      > &
        AnimaXViewProps;
    }
  }
}
// Preset canvas sizes
const sizePresets = [
  { label: 'Small', width: 200, height: 200 },
  { label: 'Medium', width: 400, height: 400 },
  { label: 'Large', width: 600, height: 600 },
];
// Preset background colors
const colorPresets = [
  { label: 'White', value: '#ffffff' },
  { label: 'Black', value: '#000000' },
  { label: 'Blue', value: '#1890ff' },
];
// Playback segment options
const segmentOptions = [
  { label: '0-30', value: [0, 30] },
  { label: '30-60', value: [30, 60] },
  { label: '60-90', value: [60, 90] },
  { label: '90-120', value: [90, 120] },
  { label: '120-150', value: [120, 150] },
  { label: '150-200', value: [150, 200] },
];
// Visibility options
const visibilityOptions = [
  { label: 'Visible', value: true },
  { label: 'Hidden', value: false },
];
// Object fit & position states
const objectFitOptions = [
  { label: 'Contain', value: 'contain' },
  { label: 'Center', value: 'center' },
  { label: 'Cover', value: 'cover' },
  { label: 'Fill', value: 'fill' },
  { label: 'Scale Down', value: 'scale-down' },
];
const objectPositionOptions = [
  { label: 'Center', value: 'center' },
  { label: 'Left', value: 'left' },
  { label: 'Right', value: 'right' },
  { label: 'Top', value: 'top' },
  { label: 'Bottom', value: 'bottom' },
  { label: 'TopLeft', value: 'top-left' },
  { label: 'TopRight', value: 'top-right' },
  { label: 'BottomLeft', value: 'bottom-left' },
  { label: 'BottomRight', value: 'bottom-right' },
];
const backendOptions = [
  { label: 'WebGPU', value: 'webgpu' },
  { label: 'WebGL', value: 'webgl' },
];

const MAX_FRAME = 200;
export default function Page() {
  // Backend state
  const [backend, setBackend] = useState<'webgpu' | 'webgl'>('webgl');
  // Animation URL states
  const [selectedUrl, setSelectedUrl] = useState(urls[0] as AnimaPageUrl);
  const [customUrl, setCustomUrl] = useState('');
  console.log('selectedUrl', selectedUrl);
  const displayUrl = customUrl || selectedUrl.url;
  const propertyButtons = customUrl ? undefined : selectedUrl.propertyButtons;
  const dynamicResource = propertyButtons?.length > 0;
  // Canvas size states
  const [width, setWidth] = useState(400);
  const [height, setHeight] = useState(400);
  const [selectedSizePreset, setSelectedSizePreset] = useState('Medium');
  // Background color state
  const [backgroundColor, setBackgroundColor] = useState('#ffffff');
  // Playback control states
  const [loop, setLoop] = useState(true);
  const [speed, setSpeed] = useState(1.0);
  const [frameRange, setFrameRange] = useState<[number, number]>([
    0,
    MAX_FRAME,
  ]);
  const [keepLastFrame, setKeepLastFrame] = useState(true);
  // Visibility & opacity
  const [visible, setVisible] = useState(true);
  const [opacity, setOpacity] = useState(1);
  // Object fit & position states
  const [objectFit, setObjectFit] = useState<
    'contain' | 'center' | 'cover' | 'fill' | 'scale-down'
  >('contain');
  const [objectPosition, setObjectPosition] = useState<
    | 'center'
    | 'left'
    | 'right'
    | 'top'
    | 'bottom'
    | 'top-left'
    | 'top-right'
    | 'bottom-left'
    | 'bottom-right'
  >('center');
  const pixelRatio =
    typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1;
  const animaXRef = useRef<any>(null);
  const lottieRef = useRef<any>(null);
  // Playback actions
  const play = () => {
    animaXRef.current?.play();
    lottieRef.current?.play();
  };
  const pause = () => {
    animaXRef.current?.pause();
    lottieRef.current?.pause();
  };
  const resume = () => {
    animaXRef.current?.resume();
    lottieRef.current?.resume();
  };
  const stop = () => {
    animaXRef.current?.stop();
    lottieRef.current?.stop();
  };
  const playSegment = (startFrame: number, endFrame: number) => {
    animaXRef.current?.playSegment(startFrame, endFrame);
    lottieRef.current?.playSegment(startFrame, endFrame);
    setFrameRange([startFrame, endFrame]);
  };
  // Canvas size preset change
  const handleSizePresetChange = (presetLabel: string) => {
    const preset = sizePresets.find((p) => p.label === presetLabel);
    if (preset) {
      setSelectedSizePreset(presetLabel);
      setWidth(preset.width);
      setHeight(preset.height);
    }
  };

  // Font loading (only once)
  useEffect(() => {
    const loadDefaultFont = async () => {
      const fontUrlNotoSansSC =
        'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/font/NotoSansSC-Regular.ttf';
      try {
        await AnimaXViewElement.loadDefaultFont(fontUrlNotoSansSC);
        console.log('Font loaded successfully');
      } catch (error) {
        console.error('Failed to load font:', error);
      }
    };
    loadDefaultFont();
  }, []);
  // Register anima-x-view events
  useEffect(() => {
    const element = animaXRef.current;
    if (!element) return;
    // Event handler functions
    const handleReady = (e: any) => {
      console.log('Animation ready', e.detail);
    };
    const handleCompletion = (e: any) =>
      console.log('Animation completed', e.detail);
    const handleStart = (e: any) => console.log('Animation started', e.detail);
    const handleRepeat = (e: any) =>
      console.log('Animation repeated', e.detail);
    const handleError = (e: any) => console.error('Animation error', e.detail);
    const handleFps = (e: any) => console.log('FPS update', e.detail);
    const handleFirstFrame = (e: any) =>
      console.log('First frame rendered', e.detail);
    const handleTapLayers = (e: any) => console.log('Tap layers', e.detail);
    const handleCompositionReady = (e: any) =>
      console.log('Composition ready', e.detail);
    const handleCancel = (e: any) =>
      console.log('Animation canceled', e.detail);
    const handleUpdate = (e: any) => console.log('Animation update', e.detail);
    element.addEventListener('ready', handleReady);
    element.addEventListener('completion', handleCompletion);
    element.addEventListener('start', handleStart);
    element.addEventListener('repeat', handleRepeat);
    element.addEventListener('error', handleError);
    element.addEventListener('fps', handleFps);
    element.addEventListener('firstframe', handleFirstFrame);
    element.addEventListener('taplayers', handleTapLayers);
    element.addEventListener('compositionready', handleCompositionReady);
    element.addEventListener('cancel', handleCancel);
    element.addEventListener('update', handleUpdate);
    return () => {
      element.removeEventListener('ready', handleReady);
      element.removeEventListener('completion', handleCompletion);
      element.removeEventListener('start', handleStart);
      element.removeEventListener('repeat', handleRepeat);
      element.removeEventListener('error', handleError);
      element.removeEventListener('fps', handleFps);
      element.removeEventListener('firstframe', handleFirstFrame);
      element.removeEventListener('taplayers', handleTapLayers);
      element.removeEventListener('compositionready', handleCompositionReady);
      element.removeEventListener('cancel', handleCancel);
      element.removeEventListener('update', handleUpdate);
    };
  }, [displayUrl]);
  return (
    <div
      style={{
        display: 'flex',
        minHeight: '100vh',
        padding: 20,
        backgroundColor: '#f5f5f5',
      }}
    >
      {/* Control Panel */}
      <div
        style={{
          flex: '0 0 360px',
          padding: 16,
          backgroundColor: '#fff',
          borderRadius: 8,
          boxShadow: '0 2px 10px rgba(0,0,0,0.08)',
          marginRight: 16,
          overflowY: 'auto',
          fontSize: 14,
        }}
      >
        <h2 style={{ marginBottom: 12 }}>Animation Controls</h2>
        {/* Animation selection and custom URL */}
        <div style={{ display: 'flex', gap: 8, marginBottom: 12 }}>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Preset Animations
            </label>
            <select
              value={selectedUrl.name}
              onChange={(e) => {
                const obj = urls.find((url) => url.name === e.target.value)!;
                setSelectedUrl(obj);
                setCustomUrl('');
              }}
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            >
              {urls.map(({ name }) => (
                <option key={name} value={name}>
                  {name}
                </option>
              ))}
            </select>
          </div>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Custom URL
            </label>
            <input
              type="text"
              value={customUrl}
              onChange={(e) => setCustomUrl(e.target.value)}
              placeholder="https://example.com/data.json"
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            />
          </div>
        </div>
        {/* Canvas size, background color, visibility in one row */}
        <div style={{ display: 'flex', gap: 8, marginBottom: 12 }}>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Canvas Size
            </label>
            <select
              value={selectedSizePreset}
              onChange={(e) => handleSizePresetChange(e.target.value)}
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            >
              {sizePresets.map(({ label }) => (
                <option key={label} value={label}>
                  {label}
                </option>
              ))}
            </select>
          </div>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Background
            </label>
            <select
              value={backgroundColor}
              onChange={(e) => setBackgroundColor(e.target.value)}
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            >
              {colorPresets.map(({ label, value }) => (
                <option key={label} value={value}>
                  {label}
                </option>
              ))}
            </select>
          </div>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Visibility
            </label>
            <select
              value={visible ? 'true' : 'false'}
              onChange={(e) => setVisible(e.target.value === 'true')}
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            >
              {visibilityOptions.map(({ label, value }) => (
                <option key={label} value={value.toString()}>
                  {label}
                </option>
              ))}
            </select>
          </div>
        </div>
        {/* Object Fit and Object Position */}
        <div style={{ display: 'flex', gap: 8, marginBottom: 12 }}>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Object Fit
            </label>
            <select
              value={objectFit}
              onChange={(e) => setObjectFit(e.target.value as any)}
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            >
              {objectFitOptions.map(({ label, value }) => (
                <option key={value} value={value}>
                  {label}
                </option>
              ))}
            </select>
          </div>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Object Position
            </label>
            <select
              value={objectPosition}
              onChange={(e) => setObjectPosition(e.target.value as any)}
              style={{
                width: '100%',
                padding: 6,
                borderRadius: 4,
                border: '1px solid #ddd',
              }}
            >
              {objectPositionOptions.map(({ label, value }) => (
                <option key={value} value={value}>
                  {label}
                </option>
              ))}
            </select>
          </div>
        </div>
        {/* Speed, Frame Range, Opacity in one row */}
        <div
          style={{
            display: 'flex',
            gap: 8,
            marginBottom: 12,
            alignItems: 'center',
          }}
        >
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Speed: {speed.toFixed(1)}x
            </label>
            <Slider
              min={0.1}
              max={5}
              step={0.1}
              value={speed}
              onChange={(value) => setSpeed(value as number)}
            />
          </div>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Frame: {frameRange[0]} - {frameRange[1]}
            </label>
            <Slider
              range
              min={0}
              max={MAX_FRAME}
              value={frameRange}
              onChange={(val) => setFrameRange(val as [number, number])}
            />
          </div>
          <div style={{ flex: 1 }}>
            <label
              style={{ fontWeight: 500, marginBottom: 4, display: 'block' }}
            >
              Opacity: {opacity.toFixed(2)}
            </label>
            <Slider
              min={0}
              max={1}
              step={0.01}
              value={opacity}
              onChange={(value) => setOpacity(value as number)}
            />
          </div>
        </div>
        {/* Loop and Keep Last Frame */}
        <div style={{ display: 'flex', gap: 8, marginBottom: 12 }}>
          <div style={{ flex: 1 }}>
            <label style={{ fontWeight: 500, marginRight: 8 }}>Loop</label>
            <input
              type="checkbox"
              checked={loop}
              onChange={(e) => setLoop(e.target.checked)}
            />
          </div>
          <div style={{ flex: 1 }}>
            <label style={{ fontWeight: 500, marginRight: 8 }}>
              Keep Last Frame
            </label>
            <input
              type="checkbox"
              checked={keepLastFrame}
              onChange={(e) => setKeepLastFrame(e.target.checked)}
            />
          </div>
        </div>
        {/* Playback Actions */}
        <div style={{ marginBottom: 12 }}>
          <label style={{ fontWeight: 500, display: 'block', marginBottom: 4 }}>
            Playback Actions
          </label>
          <div style={{ display: 'flex', gap: 8, flexWrap: 'wrap' }}>
            <button
              onClick={play}
              style={{
                padding: '6px 10px',
                backgroundColor: '#52c41a',
                color: 'white',
                border: 'none',
                borderRadius: 4,
                cursor: 'pointer',
              }}
            >
              ▶ Play
            </button>
            <button
              onClick={pause}
              style={{
                padding: '6px 10px',
                backgroundColor: '#faad14',
                color: 'white',
                border: 'none',
                borderRadius: 4,
                cursor: 'pointer',
              }}
            >
              ⏸ Pause
            </button>
            <button
              onClick={resume}
              style={{
                padding: '6px 10px',
                backgroundColor: '#1890ff',
                color: 'white',
                border: 'none',
                borderRadius: 4,
                cursor: 'pointer',
              }}
            >
              ⏯ Resume
            </button>
            <button
              onClick={stop}
              style={{
                padding: '6px 10px',
                backgroundColor: '#ff4d4f',
                color: 'white',
                border: 'none',
                borderRadius: 4,
                cursor: 'pointer',
              }}
            >
              ⏹ Stop
            </button>
          </div>
        </div>
        {/* Play Segment Dropdown */}
        <div style={{ marginBottom: 12 }}>
          <label style={{ fontWeight: 500, display: 'block', marginBottom: 4 }}>
            Segment
          </label>
          <select
            style={{
              width: '100%',
              padding: 6,
              borderRadius: 4,
              border: '1px solid #ddd',
            }}
            onChange={(e) => {
              const selected = segmentOptions.find(
                (opt) => opt.label === e.target.value
              );
              if (selected) {
                const [start, end] = selected.value;
                playSegment(start, end);
              }
            }}
            defaultValue=""
          >
            <option value="">Select segment</option>
            {segmentOptions.map(({ label }) => (
              <option key={label} value={label}>
                {label}
              </option>
            ))}
          </select>
        </div>
      </div>
      {/* Right Panel: Animation Preview */}
      <div
        style={{
          flex: 1,
          display: 'flex',
          gap: 20,
          backgroundColor,
          borderRadius: 8,
          boxShadow: '0 2px 10px rgba(0,0,0,0.1)',
          padding: 20,
          overflow: 'hidden',
        }}
      >
        {/* AnimaXView */}
        <div
          style={{
            flex: 1,
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            borderRight: '1px solid #ddd',
            paddingRight: 10,
          }}
        >
          <h3>
            AnimaXView
            {backendOptions.map(({ label, value }) => (
              <label key={value} style={{ marginRight: 16 }}>
                <input
                  type="radio"
                  name="backend"
                  value={value}
                  checked={backend === value}
                  onChange={() => setBackend(value as any)}
                  style={{ marginRight: 6 }}
                />
                {label}
              </label>
            ))}
          </h3>
          <animax-view
            backend={backend}
            ref={animaXRef}
            src={displayUrl}
            width={width * pixelRatio}
            height={height * pixelRatio}
            loop={loop}
            speed={speed}
            start-frame={frameRange[0]}
            end-frame={frameRange[1]}
            keeplastframe={keepLastFrame}
            objectfit={objectFit}
            object-position={objectPosition}
            dynamic-resource={dynamicResource}
            style={{
              display: visible ? 'block' : 'none',
              opacity,
              transition: 'opacity 0.3s',
              width: width,
              height: height,
              border: '1px dashed #ddd',
            }}
          />
          {dynamicResource && (
            <div style={{ display: 'block', gap: 8, flexWrap: 'wrap' }}>
              <label style={{ fontWeight: 500, margin: 4, display: 'block' }}>
                Dynamic Layer or Resource Properties
              </label>
              {(selectedUrl.propertyButtons ?? []).map((button, idx) => {
                const dataArr = Array.isArray(button.data)
                  ? button.data
                  : [button.data];
                return (
                  <button
                    key={`${button.label}-${button.is_resource}-${idx}`}
                    onClick={() => {
                      dataArr.forEach((item) => {
                        if (button.is_resource ?? false) {
                          animaXRef.current?.setResourceProperty(
                            item.type,
                            item.key ?? '', // resource_id
                            item.value,
                            (success: boolean, errorType: number) => {
                              console.log(
                                'DynamicResourceProperty',
                                success,
                                errorType
                              );
                            }
                          );
                        } else {
                          animaXRef.current?.updateLayerProperty(
                            item.type,
                            item.key ?? '**', // layer_name
                            item.value,
                            (success: boolean, errorType: number) => {
                              console.log(
                                'DynamicLayerProperty',
                                success,
                                errorType
                              );
                            }
                          );
                        }
                      });
                      animaXRef.current?.play();
                    }}
                    style={{
                      margin: '6px 10px',
                      padding: '6px 10px',
                      backgroundColor: '#52c41a',
                      color: 'white',
                      border: 'none',
                      borderRadius: 4,
                      cursor: 'pointer',
                    }}
                  >
                    {button.label}
                  </button>
                );
              })}
            </div>
          )}
        </div>
        {/* LottieWeb */}
        <div
          style={{
            flex: 1,
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            paddingLeft: 10,
          }}
        >
          <h3>LottieWeb</h3>
          <LottieView
            ref={lottieRef}
            src={displayUrl}
            width={width * pixelRatio}
            height={height * pixelRatio}
            loop={loop}
            speed={speed}
            startFrame={frameRange[0]}
            endFrame={frameRange[1]}
            keepLastFrame={keepLastFrame}
            objectFit={objectFit}
            objectPosition={objectPosition}
            dynamicResource={dynamicResource}
            visible={visible}
            style={{
              opacity,
              transition: 'opacity 0.3s',
              width: width,
              height: height,
              border: '1px dashed #ddd',
            }}
          />
        </div>
      </div>
    </div>
  );
}
