'use strict'

const EventEmitter = require('events');
const util = require('util')

var instances = {}

module.exports = (client) => {

  // class InputDialog
  function InputDialog() {
    this.id = null
    this.validateFunc = null
    this.labelText = null
    this.textValue = null
  }

  util.inherits(InputDialog, EventEmitter);

  // static members

  InputDialog.getInstance = function(id) {
    return id in instances ? instances[id] : null
  }

  InputDialog.setInstance = function(id, instance) {
    if (id != null) {
      instances[id] = instance
    }
  }

  InputDialog.removeInstance = function(id) {
    delete instances[id]
  }

  // instance members

  InputDialog.prototype.textValueChanged = function(text) {
    // Validate input
    if (this.validateFunc != null) {
      const id = this.id
      this.validateFunc(text, (result) => {
        if (result) {
          client.notify('enableOK', id)
        } else {
          client.notify('disableOK', id)
        }
      })
    }
  }

  InputDialog.prototype.show = function() {
    const id = client.invoke('newInputDialog', -1)
    this.id = id
    InputDialog.setInstance(id, this)
    
    if (this.labelText != null) {
      client.notify('setLabelText', this.id, this.labelText)
    }
    if (this.textValue != null) {
      client.notify('setTextValue', this.id, this.textValue)
    }

    const text = client.invoke('show', this.id)
    
    // clean up
    client.notify('deleteLater', this.id)
    InputDialog.removeInstance(id)
    
    return text
  }

  return InputDialog
}