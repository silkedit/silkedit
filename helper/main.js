'use strict'

if (process.argv.length < 3) {
  console.log('missing argument.');
  return;
}

// cache silkedit module to share it globaly
require('silkedit')