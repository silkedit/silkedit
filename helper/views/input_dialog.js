'use strict'

var EventEmitter = require('events').EventEmitter;
var util = require('util')

var instances = {}

module.exports = (client) => {

  // class InputDialog
  const InputDialog = () => {
    this.id = null
    this.validateFunc = null
    this.labelText = null
    this.textValue = null
  }

  util.inherits(InputDialog, EventEmitter);

  // static members

  InputDialog.getInstance = (id) => {
    return id in instances ? instances[id] : null
  }

  InputDialog.setInstance = (id, instance) => {
    if (id != null) {
      instances[id] = instance
    }
  }

  InputDialog.removeInstance = (id) => {
    delete instances[id]
  }

  // instance members

  InputDialog.prototype.textValueChanged = (text) => {
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

  InputDialog.prototype.show = () => {
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