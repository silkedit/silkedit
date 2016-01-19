'use strict'

const path = require('path')

// add builtin node_modules path to NODE_PATH to load silkedit module globally
if (process.env.NODE_PATH) {
  process.env.NODE_PATH = path.join(__dirname, 'node_modules') + ':' + process.env.NODE_PATH
} else {
  process.env.NODE_PATH = path.join(__dirname, 'node_modules')
}

require('module').Module._initPaths();

if (process.argv.length < 2) {
  console.log('missing argument.');
  return;
}

// cache silkedit module to share it globaly
require('silkedit')