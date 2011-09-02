var source = new EventSource("events");

function makeEventListener(f) {
  return function (e) {
    f(JSON.parse(e.data));
  };
}

source.addEventListener("message", makeEventListener(function(data) {
                          //document.getElementById("d").innerHTML += e.data + "<br>";
                        }));

source.addEventListener("chanmsg", 
                        makeEventListener(
                          function(data) {
                            var n = networks[data.network];
                            var c = n.channels[data.channel];
                            //console.log("chanmsg: " + n + ", " + c);
                            c.OnMessage(data.user, data.msg, data.action, data.self);
                            updateChannel(c, n);
                          }));

source.addEventListener("privmsg",
                        makeEventListener(
                          function(data) {
                            networks[data.network].OnPrivmsg(data.user, data.msg,
                                                             data.action, data.self);
                          }));

source.addEventListener("op", makeEventListener(function(data) {

                        }));

source.addEventListener("voice", makeEventListener(function(data) {

                        }));

source.addEventListener("kick", makeEventListener(function(data) {

                        }));

source.addEventListener("quit", makeEventListener(function(data) {

                        }));

source.addEventListener("join", makeEventListener(function(data) {

                        }));

source.addEventListener("part", makeEventListener(function(data) {

                        }));

source.addEventListener("nick", makeEventListener(function(data) {

                        }));

source.addEventListener("topic", makeEventListener(function(data) {

                        }));
