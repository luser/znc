var source = new EventSource("events");
source.addEventListener("message", function(e) {
                          document.getElementById("d").innerHTML += e.data + "<br>";
                        });

source.addEventListener("chanmsg", function(e) {
                          document.getElementById("d").innerHTML += "chanmsg: " + e.data + "<br>";
                        });

source.addEventListener("privmsg", function(e) {
                          document.getElementById("d").innerHTML += "privmsg: " + e.data + "<br>";
                        });

source.addEventListener("op", function(e) {
                          document.getElementById("d").innerHTML += "op: " + e.data + "<br>";
                        });

source.addEventListener("voice", function(e) {
                          document.getElementById("d").innerHTML += "voice: " + e.data + "<br>";
                        });

source.addEventListener("kick", function(e) {
                          document.getElementById("d").innerHTML += "kick: " + e.data + "<br>";
                        });

source.addEventListener("quit", function(e) {
                          document.getElementById("d").innerHTML += "quit: " + e.data + "<br>";
                        });

source.addEventListener("join", function(e) {
                          document.getElementById("d").innerHTML += "join: " + e.data + "<br>";
                        });

source.addEventListener("part", function(e) {
                          document.getElementById("d").innerHTML += "part: " + e.data + "<br>";
                        });

source.addEventListener("nick", function(e) {
                          document.getElementById("d").innerHTML += "nick: " + e.data + "<br>";
                        });

source.addEventListener("topic", function(e) {
                          document.getElementById("d").innerHTML += "topic: " + e.data + "<br>";
                        });
