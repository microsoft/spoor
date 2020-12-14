// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

module.exports = {
  roots: ['<rootDir>/src'],
  transform: {
    '^.*\\.(ts|tsx)$': 'ts-jest',
  },
  collectCoverage: true,
};
