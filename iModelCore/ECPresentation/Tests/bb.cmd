@echo off
%USERPROFILE%\.local\bin\uv run --frozen --directory "%SrcRoot%bentleybuild" bentleybuild.py %*
