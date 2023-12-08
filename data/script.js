function saveApiKeyAndFetchStatus() {
    saveApiKey();
    getCurrentStatus();
}

function saveApiKey() {
    const apiKey = document.getElementById('api-key').value;
    localStorage.setItem('api-key', apiKey);
}

function adjustTemperature(change) {
    const apiKey = document.getElementById('api-key').value;
    if (!apiKey) {
        alert('API Key is required');
        return;
    }

    let currentTemp = parseFloat(document.getElementById('current-temperature').textContent);
    let newTemp = (!isNaN(currentTemp) ? currentTemp + change : change).toFixed(2);
    sendData('/api/v1/temperature', { value: newTemp });
}

// 新增函数以获取当前状态
function getCurrentStatus() {
    const apiKey = document.getElementById('api-key').value;
    if (!apiKey) {
        alert('Please enter and save your API key first.');
        return;
    }

    ['temperature', 'fan'].forEach(type => {
        fetch(`/api/v1/${type}`, {
            method: 'GET',
            headers: { 'api-key': document.getElementById('api-key').value }
        })
            .then(response => response.json())
            .then(data => {
                if (data.code === "200" && data.message === "success") {
                    if (type === 'temperature') {
                        document.getElementById('current-temperature').textContent = data.data.temperature;
                    } else if (type === 'fan') {
                        document.getElementById('current-fan-mode').textContent = data.data.mode;
                    }
                }
            })
            .catch(error => console.error('Error:', error));
    });
}

function formatJson(json) {
    if (typeof json !== 'string') {
        json = JSON.stringify(json, null, 2);
    }

    json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    json = json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*")(\s*:)?:/g, (match) => {
        return `<span class="json-key">${match}</span>`;
    }).replace(/: "(.*?)"/g, (match, value) => {
        return `: <span class="json-string">"${value}"</span>`;
    }).replace(/: (true|false|null)/g, (match, value) => {
        return `: <span class="json-value">${value}</span>`;
    }).replace(/: (\d+)/g, (match, value) => {
        return `: <span class="json-value">${value}</span>`;
    });

    return `<pre class="json">${json}</pre>`;
}

function sendData(endpoint, data) {
    saveApiKey();
    const apiKey = document.getElementById('api-key').value;
    if (!apiKey) {
        alert('API Key is required');
        return;
    }

    const formData = new URLSearchParams();
    for (const key in data) {
        formData.append(key, data[key]);
    }

    fetch(endpoint, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
            'api-key': apiKey
        },
        body: formData
    })
        .then(response => response.json())
        .then(data => {
            document.getElementById('response').innerHTML = formatJson(data);
            if (data.code === "200") {
                getCurrentStatus(); // 更新数据后刷新状态显示
            }
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

// 页面加载时，检查并填充 API 密钥
window.onload = function () {
    const savedApiKey = localStorage.getItem('api-key');
    if (savedApiKey) {
        document.getElementById('api-key').value = savedApiKey;
        getCurrentStatus();
    }
}