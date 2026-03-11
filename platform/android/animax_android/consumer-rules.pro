-keepclasseswithmembers class * {
    @com.lynx.animax.base.CalledByNative <methods>;
}

-keep @androidx.annotation.Keep class *
-keepclassmembers class * {
    @androidx.annotation.Keep *;
}
