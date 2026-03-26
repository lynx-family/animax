import React, {
  useRef,
  useEffect,
  useImperativeHandle,
  forwardRef,
} from 'react';
import lottie from 'lottie-web';
export type LottieViewProps = {
  src: string;
  width: number;
  height: number;
  loop?: boolean;
  speed?: number;
  startFrame?: number;
  endFrame?: number;
  keepLastFrame?: boolean;
  objectFit?: string;
  objectPosition?: string;
  dynamicResource?: boolean;
  style?: React.CSSProperties;
  visible?: boolean;
  opacity?: number;
};
export type LottieViewHandle = {
  play: () => void;
  pause: () => void;
  stop: () => void;
  resume: () => void;
  playSegment: (start: number, end: number) => void;
};
export const LottieView = forwardRef<LottieViewHandle, LottieViewProps>(
  (
    {
      src,
      width,
      height,
      loop = true,
      speed = 1.0,
      startFrame = 0,
      endFrame,
      keepLastFrame = false,
      objectFit = 'contain',
      objectPosition = 'center',
      dynamicResource = false,
      style = {},
      visible = true,
      opacity = 1,
    },
    ref
  ) => {
    const containerRef = useRef<HTMLDivElement>(null);
    const animRef = useRef<any>(null);
    useEffect(() => {
      if (!src || !containerRef.current) return;
      let destroyed = false;
      fetch(src)
        .then((res) => res.json())
        .then((data) => {
          if (destroyed) return;
          if (data.assets) {
            const srcDir = src.substring(0, src.lastIndexOf('/') + 1);
            data.assets.forEach((asset: any) => {
              if (asset.p && /^data:/.test(asset.p)) {
                return;
              }
              if (asset.u && !/^https?:\/\//.test(asset.u)) {
                asset.u = srcDir + asset.u;
              } else if (!asset.u) {
                asset.u = srcDir;
              }
            });
          }
          animRef.current = lottie.loadAnimation({
            container: containerRef.current!,
            renderer: 'svg',
            loop,
            autoplay: visible,
            animationData: data,
            rendererSettings: {
              preserveAspectRatio:
                objectFit === 'cover' ? 'xMidYMid slice' : 'xMidYMid meet',
            },
          });
          animRef.current.setSpeed(speed);
          animRef.current.goToAndStop(startFrame, true);
          let stopped = false;
          const handleEnterFrame = (e: any) => {
            if (stopped) return;
            if (endFrame !== undefined && e.currentTime >= endFrame) {
              stopped = true;
              if (keepLastFrame) {
                animRef.current.goToAndStop(endFrame, true);
              } else {
                animRef.current.stop();
              }
            }
          };
          animRef.current.addEventListener('enterFrame', handleEnterFrame);
          if (!visible) {
            animRef.current.stop();
          } else {
            animRef.current.play();
          }
          return () => {
            animRef.current?.removeEventListener(
              'enterFrame',
              handleEnterFrame
            );
          };
        });
      return () => {
        destroyed = true;
        animRef.current?.destroy();
        animRef.current = null;
      };
    }, [
      src,
      loop,
      speed,
      startFrame,
      endFrame,
      keepLastFrame,
      objectFit,
      visible,
    ]);
    useImperativeHandle(ref, () => ({
      play: () => animRef.current?.play(),
      pause: () => animRef.current?.pause(),
      stop: () => animRef.current?.stop(),
      resume: () => animRef.current?.play(),
      playSegment: (start: number, end: number) =>
        animRef.current?.playSegments([start, end], true),
    }));
    const composedStyle: React.CSSProperties = {
      width,
      height,
      objectFit,
      objectPosition,
      visibility: visible ? 'visible' : 'hidden',
      opacity,
      transition: 'opacity 0.3s',
      ...style,
    };
    return (
      <div
        ref={containerRef}
        style={{
          ...composedStyle,
          display: visible ? 'block' : 'none',
          border: '1px dashed #ddd',
        }}
      />
    );
  }
);
