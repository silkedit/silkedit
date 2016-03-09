'use strict';  const fs = require('fs');
const path = require('path');

module.exports = (grunt) => {
  grunt.registerTask('generate-license', 'Generate the license, including the licenses of all dependencies', function(mode) {
    const legalEagle = require('legal-eagle');
    const done = this.async();

    const options = {
      path: path.join(process.cwd(), "jslib/node_modules/silkedit"),
      overrides: require('./license-overrides')
    }

    // licenses in jslib
    legalEagle(options, (err, jslibDependencyLicenses) => {
      if (err != null) {
        console.error(err);
        process.exit(1);
      }


      var text = `
${fs.readFileSync('LICENSE.md', 'utf8')}

This application bundles the following third-party packages in accordance
with the following licenses:\n\n
`;
      
      const packagesOptions = {
        path: path.join(process.cwd(), 'packages'),
        overrides: require('./license-overrides')
      }

      // licenses in bundled packages
      legalEagle(packagesOptions, (err, dependencyLicenses) => {
        if (err != null) {
          console.error(err);
          process.exit(1);
        }
        text += getLicenseText(Object.assign(dependencyLicenses, jslibDependencyLicenses));
      
        if (mode === 'save') {
          const targetPath = 'resources/LICENSE.md';
          fs.writeFileSync(targetPath, text);
        } else {
          console.log(text);
        }
        
        done();
      });
    });
  });
};

function isPublic(name) {
  return !name.startsWith('default@') && !name.startsWith('silkedit@') && !name.startsWith('silkedit-packages@');
}

function getLicenseText(dependencyLicenses) {
  const _ = require('underscore-plus');
  var text = '';
  const names = _.keys(dependencyLicenses).filter(isPublic).sort();
  for (let name of names) {

    const info = dependencyLicenses[name];
    const license = info.license;
    const source = info.source;
    const sourceText = info.sourceText;

    text += "-------------------------------------------------------------------------\n\n";
    text += `Package: ${name}\n`;
    text += `License: ${license}\n`;
    if (source != null) {
      text += `License Source: ${source}\n`;
    }
    if (sourceText != null) {
      text += "Source Text:\n\n";
      text += sourceText;
    }
    text += '\n';
  }
  return text;
}