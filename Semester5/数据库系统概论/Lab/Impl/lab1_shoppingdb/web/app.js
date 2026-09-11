// 全局变量
let productsData = [];
let transactionsData = [];
let stockChart = null;
let salesChart = null;

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', function() {
    loadData();
});

// 加载数据
async function loadData() {
    showLoading(true);
    try {
        await Promise.all([
            loadProducts(),
            loadTransactions()
        ]);

        updateDashboard();
        renderCharts();
        hideLoading();
    } catch (error) {
        console.error('数据加载失败:', error);
        alert('数据加载失败，请检查数据文件是否存在');
        hideLoading();
    }
}

// 加载商品数据
async function loadProducts() {
    try {
        const response = await fetch('../data/products.txt');
        const text = await response.text();

        productsData = [];
        const lines = text.trim().split('\n');

        for (const line of lines) {
            if (line.trim()) {
                const parts = line.trim().split(' ');
                if (parts.length >= 6) {
                    productsData.push({
                        id: parseInt(parts[0]),
                        name: parts[1],
                        category: parts[2],
                        price: parseFloat(parts[3]),
                        stock: parseInt(parts[4]),
                        isDeleted: parts[5] === '1'
                    });
                }
            }
        }

        // 过滤掉已删除的商品
        productsData = productsData.filter(product => !product.isDeleted);

    } catch (error) {
        console.error('加载商品数据失败:', error);
        throw error;
    }
}

// 加载交易数据
async function loadTransactions() {
    try {
        const response = await fetch('../data/transactions.txt');
        const text = await response.text();

        transactionsData = [];
        const lines = text.trim().split('\n');

        for (const line of lines) {
            if (line.trim()) {
                const parts = line.trim().split(' ');
                if (parts.length >= 6) {
                    // 处理时间戳（可能包含空格）
                    const timestampStart = 4;
                    const timestamp = parts.slice(timestampStart, timestampStart + 2).join(' ');

                    transactionsData.push({
                        id: parseInt(parts[0]),
                        productId: parseInt(parts[1]),
                        type: parts[2],
                        operator: parts[3],
                        timestamp: timestamp,
                        quantity: parseInt(parts[parts.length - 1])
                    });
                }
            }
        }

        // 按时间排序（最新的在前）
        transactionsData.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));

    } catch (error) {
        console.error('加载交易数据失败:', error);
        throw error;
    }
}

// 更新仪表板
function updateDashboard() {
    updateStatsCards();
    updateCategoryOverview();
    updateProductsTable();
    updateTransactionList();
}

// 更新统计卡片
function updateStatsCards() {
    const totalProducts = productsData.length;
    const totalStock = productsData.reduce((sum, product) => sum + product.stock, 0);
    const totalValue = productsData.reduce((sum, product) => sum + (product.price * product.stock), 0);
    const totalTransactions = transactionsData.length;

    document.getElementById('total-products').textContent = totalProducts;
    document.getElementById('total-stock').textContent = totalStock;
    document.getElementById('total-value').textContent = `¥${totalValue.toFixed(2)}`;
    document.getElementById('total-transactions').textContent = totalTransactions;
}

// 更新分类概览
function updateCategoryOverview() {
    const categoryMap = new Map();

    productsData.forEach(product => {
        if (!categoryMap.has(product.category)) {
            categoryMap.set(product.category, {
                name: product.category,
                count: 0,
                totalStock: 0,
                totalValue: 0
            });
        }

        const category = categoryMap.get(product.category);
        category.count++;
        category.totalStock += product.stock;
        category.totalValue += product.price * product.stock;
    });

    const categoryOverview = document.getElementById('category-overview');
    categoryOverview.innerHTML = '';

    for (const [, category] of categoryMap) {
        const categoryElement = document.createElement('div');
        categoryElement.className = 'category-item';
        categoryElement.innerHTML = `
            <h3>${category.name}</h3>
            <div class="category-stats">
                <div>${category.count} 种商品</div>
                <div>${category.totalStock} 件库存</div>
                <div>¥${category.totalValue.toFixed(2)}</div>
            </div>
        `;
        categoryOverview.appendChild(categoryElement);
    }
}

// 更新商品表格
function updateProductsTable() {
    const tbody = document.getElementById('products-tbody');
    tbody.innerHTML = '';

    productsData.forEach(product => {
        const row = document.createElement('tr');

        const stockStatus = getStockStatus(product.stock);
        const stockStatusClass = getStockStatusClass(product.stock);

        row.innerHTML = `
            <td>${product.id}</td>
            <td>${product.name}</td>
            <td>${product.category}</td>
            <td>¥${product.price.toFixed(2)}</td>
            <td>${product.stock}</td>
            <td><span class="stock-status ${stockStatusClass}">${stockStatus}</span></td>
        `;

        tbody.appendChild(row);
    });
}

// 获取库存状态文本
function getStockStatus(stock) {
    if (stock === 0) return '缺货';
    if (stock <= 5) return '库存不足';
    if (stock <= 20) return '库存适中';
    return '库存充足';
}

// 获取库存状态样式类
function getStockStatusClass(stock) {
    if (stock === 0) return 'stock-out';
    if (stock <= 5) return 'stock-low';
    if (stock <= 20) return 'stock-medium';
    return 'stock-high';
}

// 更新交易记录列表
function updateTransactionList() {
    const transactionList = document.getElementById('transaction-list');
    const filter = document.getElementById('transaction-filter').value;

    let filteredTransactions = transactionsData;
    if (filter !== 'all') {
        filteredTransactions = transactionsData.filter(t => t.type === filter);
    }

    // 只显示最近20条记录
    const recentTransactions = filteredTransactions.slice(0, 20);

    transactionList.innerHTML = '';

    recentTransactions.forEach(transaction => {
        const product = productsData.find(p => p.id === transaction.productId);
        const productName = product ? product.name : `商品ID: ${transaction.productId}`;

        const transactionElement = document.createElement('div');
        transactionElement.className = `transaction-item ${transaction.type === '销售' ? 'sale' : 'restock'}`;

        transactionElement.innerHTML = `
            <div class="transaction-header">
                <span class="transaction-type ${transaction.type === '销售' ? 'sale' : 'restock'}">
                    ${transaction.type}
                </span>
                <span class="transaction-time">${formatDateTime(transaction.timestamp)}</span>
            </div>
            <div class="transaction-details">
                <strong>${productName}</strong> - 数量: ${transaction.quantity} - 操作人: ${transaction.operator}
            </div>
        `;

        transactionList.appendChild(transactionElement);
    });
}

// 格式化日期时间
function formatDateTime(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
    });
}

// 渲染图表
function renderCharts() {
    renderStockChart();
    renderSalesChart();
}

// 渲染库存状态图表
function renderStockChart() {
    const ctx = document.getElementById('stockChart').getContext('2d');

    const categories = [...new Set(productsData.map(p => p.category))];
    const stockData = categories.map(category => {
        return productsData
            .filter(p => p.category === category)
            .reduce((sum, p) => sum + p.stock, 0);
    });

    if (stockChart) {
        stockChart.destroy();
    }

    stockChart = new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: categories,
            datasets: [{
                data: stockData,
                backgroundColor: [
                    '#FF6384',
                    '#36A2EB',
                    '#FFCE56',
                    '#4BC0C0',
                    '#9966FF',
                    '#FF9F40'
                ],
                borderWidth: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'bottom'
                },
                title: {
                    display: true,
                    text: '各类别库存分布'
                }
            }
        }
    });
}

// 渲染销量趋势图表
function renderSalesChart() {
    const ctx = document.getElementById('salesChart').getContext('2d');

    // 统计最近7天的销量
    const salesData = [];
    const labels = [];

    for (let i = 6; i >= 0; i--) {
        const date = new Date();
        date.setDate(date.getDate() - i);
        const dateStr = date.toISOString().split('T')[0];

        labels.push(date.toLocaleDateString('zh-CN', { month: '2-digit', day: '2-digit' }));

        const daySales = transactionsData
            .filter(t => t.type === '销售' && t.timestamp.startsWith(dateStr))
            .reduce((sum, t) => sum + t.quantity, 0);

        salesData.push(daySales);
    }

    if (salesChart) {
        salesChart.destroy();
    }

    salesChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: '销量',
                data: salesData,
                borderColor: '#36A2EB',
                backgroundColor: 'rgba(54, 162, 235, 0.1)',
                borderWidth: 3,
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                title: {
                    display: true,
                    text: '最近7天销量趋势'
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: {
                        stepSize: 1
                    }
                }
            }
        }
    });
}

// 刷新数据
function refreshData() {
    loadData();
}

// 显示/隐藏加载动画
function showLoading(show) {
    const loading = document.getElementById('loading');
    loading.style.display = show ? 'flex' : 'none';
}

function hideLoading() {
    showLoading(false);
}

// 交易筛选器事件
document.getElementById('transaction-filter').addEventListener('change', function() {
    updateTransactionList();
});

// 错误处理
window.addEventListener('error', function(e) {
    console.error('JavaScript错误:', e.error);
    hideLoading();
});

// 处理fetch错误
window.addEventListener('unhandledrejection', function(e) {
    console.error('Promise拒绝:', e.reason);
    hideLoading();
});