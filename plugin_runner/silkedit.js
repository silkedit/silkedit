module.exports = function(client) {
  return {
    alert: function(msg) {
      console.log(msg);
      var start = Date.now();
      //client.invoke('add', 5, 4, function (err, response) {
      //  var end = Date.now();
      //  console.log('time:', end - start)
      //  console.log(response)
      //});
      client.notify('alert', msg);
    }
  }
}