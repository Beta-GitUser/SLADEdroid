# SLADEdroid Core

This directory contains the dependency boundary for the future portable SLADE core.

Code here must not include wxWidgets, desktop OpenGL, Android APIs, or desktop application headers.

The first extraction work intentionally starts with shared types and global error state. Existing SLADE code is migrated into this boundary incrementally; the desktop application remains untouched until each dependency has a replacement and tests.
