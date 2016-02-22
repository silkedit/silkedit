module.exports = function (grunt) {
    grunt.initConfig({

    // define source files and their destinations
    uglify: {
        files: {
            src: 'jslib_out/**/*.js',  // source files mask
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
