from pathlib import Path

source = Path('dashboard/index.html').read_text()
local_script = source[source.rfind('<script>'):source.rfind('</script>')]
render = local_script[local_script.index('function render(){'):local_script.index('\nfunction purge()')]
render = render.replace('const allowed=(i===0?', 'const allowed=online&&!busy&&(i===0?')
render = render.replace('b.disabled=!state.smoke||state.purging', 'b.disabled=!online||busy||!state.smoke||state.purging')
render = render.replace("state.purging?'Purging · airflow control is available.':''", "!online?'Connection to ESP unavailable.':state.purging?'Purging · airflow control is available.':''")
script = '''<script>
let state={fans:false,smoke:false,act:false,direction:'neutral',purging:false};
let online=false,busy=false,revision=0;
''' + render + '''
async function sync(){
 if(busy)return;const current=revision;
 try{const res=await fetch('/api/status',{cache:'no-store'});if(!res.ok)throw Error();const next=await res.json();if(current!==revision||busy)return;state=next;online=true}
 catch(e){if(current!==revision||busy)return;online=false}render();
}
async function send(c){const res=await fetch('/api/command',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({command:c})});if(!res.ok)throw Error('Command rejected');state=await res.json()}
async function action(commands){
 if(busy||!online)return;busy=true;revision++;render();
 try{for(const c of commands)await send(c)}catch(e){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();state=await r.json()}catch(_){online=false}}
 finally{busy=false;render()}
}
document.querySelector('#steps').addEventListener('click',e=>{const b=e.target.closest('[data-key]');if(!b||b.disabled)return;const key=b.dataset.key;action([key+(state[key]?'_off':'_on')])});
document.querySelectorAll('[data-dir]').forEach(b=>b.addEventListener('click',()=>{if(b.disabled)return;const upper=state.act&&(state.direction==='upper'||state.direction==='both'),lower=state.act&&(state.direction==='lower'||state.direction==='both');const u=b.dataset.dir==='upper'?!upper:upper,l=b.dataset.dir==='lower'?!lower:lower;const next=u?(l?'both':'upper'):(l?'lower':'neutral');if(next==='neutral')action(['actuation_off']);else action(state.act?[next]:['actuation_on',next])}));
sync();setInterval(sync,1000);
</script>'''
source = source[:source.rfind('<script>')] + script + '</body></html>\n'
assert ')VENTOHTML"' not in source
Path('firmware/vento8266/dashboard_embedded.h').write_text('#pragma once\n#include <pgmspace.h>\nconst char DASHBOARD_HTML[] PROGMEM = R"VENTOHTML(' + source + ')VENTOHTML";\n')
