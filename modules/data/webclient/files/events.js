var source = new EventSource("events");
source.addEventListener("message", function(e) {
                          document.getElementById("d").innerHTML += e.data + "<br>";
                        });

source.addEventListener("chanmsg", function(e) {
                          document.getElementById("d").innerHTML += "chanmsg: " + e.data + "<br>";
                        });

