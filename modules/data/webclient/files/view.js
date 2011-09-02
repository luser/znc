$(document).ready(function() {
                    for (var n in networks) {
                      for (var c in networks[n].channels) {
                        addChannel(networks[n].channels[c], networks[n]);
                      }
                    }

                    $("#tabs > section > h3").click(switchTab);
                  });

function switchTab() {
  $('#tabs section').removeClass('current');
  $(this).closest('section').addClass('current');
}

function channelID(channel, network) {
  return network.name + "-" + channel.name.substr(1);
}
function addChannel(channel, network) {
  $('#tabs section').removeClass('current');
  $('#tabs').append('<section id="' + channelID(channel, network) + '" class="current"><h3>' + channel.name + '</h3><div><ul></ul></div></section>');
}

function updateChannel(channel, network) {
  var m = channel.scrollback[channel.scrollback.length - 1];
  var content;
  if (m.type == "message") {
    content = '<li><span class="user">&lt;' + m.user + '&gt;</span> <span class="message">' + m.msg + '</span>';
  }
  else if (m.type == "action") {
    content = '<li>* <span class="user">' + m.user + '</span> <span class="message">' + m.msg + '</span>';
  }
  $('#' + channelID(channel, network) + ' ul').append(content);
}
