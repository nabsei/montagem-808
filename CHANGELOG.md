# Changelog

All notable changes to Montagem 808 are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versioning
follows [Semantic Versioning](https://semver.org/) adapted for a pre-1.0
beta:

- **PATCH** (0.x.y): bug fixes only, no behavior/feature changes
- **MINOR** (0.x.0): new features or notable user-facing changes
- **MAJOR** (1.0.0+): first stable release, then breaking changes only

## [0.2.1] - 2026-07-16

### Fixed
- `getTailLengthSeconds()` reported a flat 2.0s regardless of the Amount
  setting, but the exponential decay tuned in the shipped build can take
  noticeably longer at high Amount. A host that uses this value to decide
  how long to keep rendering after note-off (freezing a track, offline
  bounce, silence detection) would cut the tail off audibly early. Now
  reports a value that covers the worst case.

## [0.2.0] - 2026-07-15

### Added
- Resizable window (400x360 up to 900x720); the knob now scales with the
  available space instead of staying a fixed size. The on-screen keyboard
  shows more keys at wider sizes.

## [0.1.0] - 2026-07-13

### Added
- First public beta: one-knob 808 slide/glide synth with built-in on-screen
  keyboard, VST3/AU/Standalone on macOS and Windows.
