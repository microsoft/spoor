// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

const baseConfig = require('../../jest.config');
const packageJson = require('./package');

module.exports = {
  ...baseConfig,
  name: packageJson.name,
  displayName: packageJson.name,
  coveragePathIgnorePatterns: ['.*_pb.js'],
};
