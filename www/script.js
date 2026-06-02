const refreshBtn = document.getElementById('refresh-files');
const fileListContainer = document.getElementById('file-list');
const sendReqBtn = document.getElementById('send-request');
const pathInput = document.getElementById('req-path');
const methodSelect = document.getElementById('req-method');
const headersPre = document.getElementById('response-headers');
const bodyDiv = document.getElementById('response-body');

document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        const tabId = btn.getAttribute('data-tab');
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
        document.getElementById(`tab-${tabId}`).classList.add('active');
        if (tabId === 'files') refreshFileList();
    });
});

async function refreshFileList() {
    fileListContainer.innerHTML = '<li>Загрузка...</li>';
    try {
        const res = await fetch('/files');
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const files = await res.json();
        if (!files.length) {
            fileListContainer.innerHTML = '<li>Нет файлов</li>';
            return;
        }
        const ul = document.createElement('ul');
        ul.className = 'file-list-api';
        files.forEach(file => {
            const li = document.createElement('li');
            li.innerHTML = `<span>${file.name} (${file.size} bytes)</span>
                            <button class="delete-file-btn" data-path="${file.name}">Удалить</button>`;
            ul.appendChild(li);
        });
        fileListContainer.innerHTML = '';
        fileListContainer.appendChild(ul);
        // Обработчики удаления
        document.querySelectorAll('.delete-file-btn').forEach(btn => {
            btn.addEventListener('click', async (e) => {
                const filePath = btn.getAttribute('data-path');
                if (confirm(`Удалить ${filePath}?`)) {
                    await deleteFile(filePath);
                    refreshFileList();
                }
            });
        });
    } catch (err) {
        fileListContainer.innerHTML = `<li>Ошибка: ${err.message}</li>`;
    }
}

async function deleteFile(filePath) {
    try {
        const res = await fetch(`/delete?file=${encodeURIComponent(filePath)}`, { method: 'DELETE' });
        const data = await res.json();
        alert(res.ok ? `Удалено: ${data.file}` : `Ошибка: ${data.status || res.statusText}`);
    } catch (err) {
        alert(`Ошибка удаления: ${err.message}`);
    }
}

async function sendRequest(path, method) {
    if (!path) path = '/';
    if (!path.startsWith('/')) path = '/' + path;
    const url = `http://localhost:8080${path}`;
    try {
        const resp = await fetch(url, { method: method });
        const headersObj = {};
        for (let [k,v] of resp.headers.entries()) headersObj[k] = v;
        headersPre.textContent = `HTTP/${resp.status} ${resp.statusText}\n` +
            Object.entries(headersObj).map(([k,v]) => `${k}: ${v}`).join('\n');
        if (method === 'GET') {
            let text = await resp.text();
            if (text.length > 1500) text = text.slice(0,1500) + '\n... (truncated)';
            bodyDiv.innerHTML = `<strong>тело ответа (${text.length} символов)</strong><br><pre style="background:#0a0a0a; padding:0.5rem;">${escapeHtml(text)}</pre>`;
        } else {
            bodyDiv.innerHTML = `<em>⚠HEAD-запрос не возвращает тело</em>`;
        }
    } catch (err) {
        headersPre.textContent = `Ошибка: ${err.message}`;
        bodyDiv.innerHTML = '';
    }
}
sendReqBtn.addEventListener('click', () => {
    sendRequest(pathInput.value.trim(), methodSelect.value);
});

function escapeHtml(str) {
    return str.replace(/[&<>]/g, function(m) {
        if(m === '&') return '&amp;';
        if(m === '<') return '&lt;';
        if(m === '>') return '&gt;';
        return m;
    });
}

refreshFileList();