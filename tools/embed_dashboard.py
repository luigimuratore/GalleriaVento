from pathlib import Path
source = Path('dashboard/index.html').read_text()
script = '''<script>
let state={fans:false,smoke:false,act:false,direction:'neutral',purging:false};let online=false,busy=false;
const keys=['fans','smoke','act'];const items=[['ARIA'],['FUMO'],['FLUSSI']];
function render(){
 document.querySelector('#steps').innerHTML=items.map((it,i)=>{const k=keys[i],on=state[k];let allowed=on?(i===2||!state[keys[i+1]]):(i===0||state[keys[i-1]]);if((state.purging && k!=='fans')||!online||busy)allowed=false;return `<div class="step ${on?'is-on':''}"><span class="number">0${i+1}</span><div class="info"><strong>${it[0]}</strong></div><button class="toggle ${on?'active':''}" data-index="${i}" ${allowed?'':'disabled'} aria-pressed="${on}" aria-label="${on?'Spegni':'Accendi'} ${it[0]}"><svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 2v10M6.4 5.8a8 8 0 1 0 11.2 0"/></svg></button></div>`}).join('');
 document.querySelector('#progress').textContent=String(keys.filter(k=>state[k]).length).padStart(2,'0')+' / 03 ATTIVI';
 document.querySelector('.visual').classList.toggle('directed-active',state.act&&state.direction!=='neutral');document.querySelector('#airLines').classList.toggle('on',state.fans);document.querySelector('#smoke').classList.toggle('on',state.smoke);document.querySelector('#upper').classList.toggle('on',state.act&&state.direction==='upper');document.querySelector('#lower').classList.toggle('on',state.act&&state.direction==='lower');
 document.querySelector('#directionLabel').textContent=({upper:'SOPRA',lower:'SOTTO',neutral:'NEUTRO'})[state.direction];document.querySelectorAll('[data-dir]').forEach(b=>{b.disabled=!online||busy||!state.act;b.classList.toggle('selected',b.dataset.dir===state.direction);b.setAttribute('aria-pressed',String(b.dataset.dir===state.direction))});
 document.querySelector('#notice').textContent=!online?'Connessione alla scheda assente.':state.purging?'Spurgo simulato · puoi comandare ARIA.':'ESP connesso · simulazione, nessun carico collegato.';
}
async function sync(){if(busy)return;try{const res=await fetch('/api/status',{cache:'no-store'});if(!res.ok)throw 0;state=await res.json();online=true}catch(e){online=false}render()}
async function command(c){if(busy||!online)return;busy=true;render();try{const res=await fetch('/api/command',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({command:c})});state=await res.json();online=true}catch(e){online=false}finally{busy=false;render()}}
document.querySelector('#steps').addEventListener('click',e=>{const b=e.target.closest('[data-index]');if(!b||b.disabled)return;const i=+b.dataset.index,k=keys[i];command(['fans','smoke','actuation'][i]+(state[k]?'_off':'_on'))});
document.querySelectorAll('[data-dir]').forEach(b=>b.addEventListener('click',()=>command(b.dataset.dir)));document.querySelector('#reset').onclick=()=>command('reset');sync();setInterval(sync,1000);
</script>'''
source = source[:source.rfind('<script>')] + script + '</body></html>\n'
assert ')VENTOHTML"' not in source
Path('firmware/vento8266/dashboard_embedded.h').write_text('#pragma once\n#include <pgmspace.h>\nconst char DASHBOARD_HTML[] PROGMEM = R"VENTOHTML(' + source + ')VENTOHTML";\n')
