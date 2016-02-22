module.exports = function (grunt) {
    grunt.initConfig({

    // define source files and their destinations
    uglify: {
			jslib: {
            src: 'minified/jslib/**/*.js',  // source files mask
            dest: '.',    // destination folder
            expand: true,    // allow dynamic building
            flatten: false,   // remove all unnecessary nesting
            ext: '.js'   // replace .js to .min.js
			},
			defualt: {
            src: ['minified/default/index.js', 'minified/default/lib/**/*.js'],  // source files mask
            dest: '.',    // destination folder
            expand: true,    // allow dynamic building
            flatten: false,   // remove all unnecessary nesting
            ext: '.js'   // replace .js to .min.js
			}
    }
});

// load plugins
grunt.loadNpmTasks('grunt-contrib-uglify');

// register at least this one task
grunt.registerTask('default', [ 'uglify' ]);


};
