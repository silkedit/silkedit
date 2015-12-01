'use strict'

const EventEmitter = require('events');
const util = require('util')

var instances = {}

function setInstance(id, instance) {
  if (id != null) {
    instances[id] = instance
  }
}

function removeInstance(id) {
  delete instances[id]
}

module.exports = {   
  // static members

  "getInstance": (id) => {
    return id in instances ? instances[id] : null
  }
  
  ,"InputDialog": (ObjectProxy) => {

  const API = require('../core/api')(ObjectProxy)

  // class InputDialog
  function InputDialog() {
    this.id = null
    this.notifyMethods = new Set(['enableOK', 'disableOK', 'setLabelText', 'setTextValue', 'setCurrentIndex'])
    this.validateFunc = null
    this.labelText = null
    this.textValue = null
  }

  util.inherits(InputDialog, EventEmitter);

  // instance members
  
  InputDialog.prototype = Object.create(ObjectProxy)

  InputDialog.prototype.textValueChanged = function(text) {
    // Validate input
    if (this.validateFunc != null) {
      const self = this
      this.validateFunc(text, (result) => {
        if (result) {
          self.enableOK()
        } else {
          self.disableOK()
        }
      })
    }
  }

  InputDialog.prototype.show = function() {
    const id = API.newInputDialog()
    this.id = id
    setInstance(id, this)
    
    if (this.labelText != null) {
      this.setLabelText(this.labelText)
    }
    if (this.textValue != null) {
      this.setTextValue(this.textValue)
    }

    const text = this.showDialog()
    
    // clean up
    this.deleteLater()
    removeInstance(id)
    
    return text
  }

  return InputDialog
}
}