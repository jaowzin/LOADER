# Changelog

## v1.6

- Added Shell Resources Mode.
- Keeps loader resources active for Java/Unity string lookup under `com.Nobodyshot.loader`.
- Does not patch `LoadedApk.mResDir` or `LoadedApk.mSplitResDirs`.
- Does not patch `ApplicationInfo.publicSourceDir` or `splitPublicSourceDirs`.
- Still redirects code/native paths to target `com.Nobodyshot.kuboom`.
- Avoids v1.4 UID/package spoofing issue.
