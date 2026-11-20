from pathlib import Path

source = Path('dashboard/index.html').read_text()
local_script = source[source.rfind('<script>'):source.rfind('</script>')]
render = local_script[local_script.index('function render(){'):local_script.index('\nfunction purge()')]
render = render.replace('const allowed=(i===0?', 'const allowed=online&&!busy&&(i===0?')
render = render.replace('b.disabled=!state.smoke||state.purging', 'b.disabled=!online||busy||!state.smoke||state.purging')
render = render.replace("state.purging?'Purging · airflow control is available.':''", "!online?'Connection to ESP unavailable.':state.dryTestOutputs?'BENCH TEST: all loads must be disconnected.':state.valvesPhysical?'Fans, pump and valve outputs are live. Coil is disabled.':state.fansPhysical?'Fan relays and pump are live. Coil and valves are simulated.':state.purging?'Purging · airflow control is available.':''")
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
function renderDebug(){if(!timingsDirty&&typeof state.preSeconds==='number'){preTime.value=state.preSeconds;purgeTime.value=state.purgeSeconds} if(!pumpDirty&&typeof state.pumpPercent==='number'){pumpPwm.value=state.pumpPercent;document.querySelector('#pumpValue').textContent=state.pumpPercent} for(const [id,key] of [['relay1','debugRelay1'],['relay2','debugRelay2'],['pump','debugPump'],['coil','debugCoil'],['upper','debugUpper'],['lower','debugLower']]){const b=document.querySelector('[data-debug=\"'+id+'\"]');const on=!!state[key];b.setAttribute('aria-pressed',String(on));b.textContent=on?'ON':'OFF'}}
const baseRender=render;render=function(){baseRender();renderDebug()};
document.querySelector('#steps').addEventListener('click',e=>{const b=e.target.closest('[data-key]');if(!b||b.disabled)return;const key=b.dataset.key;action([key+(state[key]?'_off':'_on')])});
document.querySelectorAll('[data-dir]').forEach(b=>b.addEventListener('click',()=>{if(b.disabled)return;const upper=state.act&&(state.direction==='upper'||state.direction==='both'),lower=state.act&&(state.direction==='lower'||state.direction==='both');const u=b.dataset.dir==='upper'?!upper:upper,l=b.dataset.dir==='lower'?!lower:lower;const next=u?(l?'both':'upper'):(l?'lower':'neutral');if(next==='neutral')action(['actuation_off']);else action(state.act?[next]:['actuation_on',next])}));
window.saveFlowTimings=async(pre,purge)=>{if(!online||busy||!state.fansPhysical)throw Error();busy=true;revision++;try{const res=await fetch('/api/settings/timings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({pre:String(pre),purge:String(purge),password:'sib'})});if(!res.ok)throw Error();state=await res.json()}finally{busy=false;render()}};
window.savePumpSetting=async percent=>{if(!online||busy||!state.fansPhysical||!state.pumpPwmReady)throw Error();busy=true;revision++;try{const res=await fetch('/api/settings/pump',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({percent:String(percent),password:'sib'})});if(!res.ok)throw Error();state=await res.json()}finally{busy=false;render()}};
window.debugValve=async(valve,on)=>{if(!online||busy||!state.valvesPhysical)throw Error();busy=true;revision++;try{const res=await fetch('/api/debug/valve',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({valve,value:on?'on':'off',password:'sib'})});if(!res.ok)throw Error();state=await res.json()}finally{busy=false;render()}};
window.debugMos=async(mos,on)=>{if(!online||busy||!state.fansPhysical)throw Error();busy=true;revision++;try{const res=await fetch('/api/debug/mos',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({mos,value:on?'on':'off',password:'sib'})});if(!res.ok)throw Error();state=await res.json()}finally{busy=false;render()}};
window.debugRelay=async(relay,on)=>{if(!online||busy||!state.fansPhysical)throw Error();busy=true;revision++;try{const res=await fetch('/api/debug/relay',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({relay,value:on?'on':'off',password:'sib'})});if(!res.ok)throw Error();state=await res.json()}finally{busy=false;render()}};sync();setInterval(sync,1000);
</script>'''
source = source[:source.rfind('<script>')] + script + '</body></html>\n'
assert ')VENTOHTML"' not in source
Path('firmware/vento8266/dashboard_embedded.h').write_text('#pragma once\n#include <pgmspace.h>\nconst char DASHBOARD_HTML[] PROGMEM = R"VENTOHTML(' + source + ')VENTOHTML";\n')
