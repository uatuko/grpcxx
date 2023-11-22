# grpcxx

[![license](https://img.shields.io/badge/license-MIT-green)](https://raw.githubusercontent.com/uatuko/grpcxx/main/LICENSE)

🚀 Blazing fast gRPC C++ Server implemented using modern standards (C++20).

## Features
* Fast - More than 2x faster compared to the official implementation(s).
* Simple - Overall smaller codebase and 95% less generated code compared to the official implementation.
* Flexible - The application code is given greater implementation choice by using C++ concepts instead of being restricted to one choice.

## Benchmarks

> Please refer to https://github.com/uatuko/grpcxx/pull/2 for detailed benchmark results.

|                               | 1a  | 1b   | 2a  | 2b   | 3a   | 3b   |
| ----------------------------- | --- | ---- | --- | ---- | ---- | ---- |
| gRPC v1.48.4 (callback)       | 25k | 87k  | 76k | **152k** | 96k  | 142k |
| grpc-go v1.56.2               | 27k | 103k | 94k | 191k | 90k  | **308k** |
| grpcxx (hardware concurrency) | 31k | 149k | 89k | **346k** | 74k  | 337k |
| grpcxx (2 workers)            | 31k | 143k | 98k | 341k | 99k  | **355k** |
