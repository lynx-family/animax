-keep @androidx.annotation.Keep class *
-keepclassmembers class * {
    @androidx.annotation.Keep *;
}

-keep @com.google.auto.service.AutoService class * {
    *;
}
