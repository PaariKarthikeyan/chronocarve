// ChronoCarve Chart.js Visualizations for Latency and Megapixels Throughput

let runtimeChart = null;
let throughputChart = null;

const defaultChartColors = {
    greedy: { bg: 'rgba(244, 63, 94, 0.75)', border: '#f43f5e' },
    backward_dp: { bg: 'rgba(245, 158, 11, 0.75)', border: '#f59e0b' },
    forward_dp: { bg: 'rgba(6, 182, 212, 0.75)', border: '#06b6d4' },
    graph_cut: { bg: 'rgba(16, 185, 129, 0.75)', border: '#10b981' }
};

function initCharts() {
    const canvasRuntime = document.getElementById('chart-runtime');
    const canvasThroughput = document.getElementById('chart-throughput');
    if (!canvasRuntime || !canvasThroughput) return;

    const ctxRuntime = canvasRuntime.getContext('2d');
    const ctxThroughput = canvasThroughput.getContext('2d');

    const commonOptions = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: { display: false },
            tooltip: {
                backgroundColor: 'rgba(15, 23, 42, 0.95)',
                titleFont: { family: 'Outfit', size: 12 },
                bodyFont: { family: 'JetBrains Mono', size: 11 },
                borderColor: 'rgba(255, 255, 255, 0.1)',
                borderWidth: 1,
                padding: 10
            }
        },
        scales: {
            x: {
                grid: { color: 'rgba(255, 255, 255, 0.05)' },
                ticks: { color: '#94a3b8', font: { family: 'Outfit', size: 11 } }
            },
            y: {
                beginAtZero: true,
                grid: { color: 'rgba(255, 255, 255, 0.05)' },
                ticks: { color: '#94a3b8', font: { family: 'JetBrains Mono', size: 10 } }
            }
        }
    };

    // Runtime Chart (ms)
    runtimeChart = new Chart(ctxRuntime, {
        type: 'bar',
        data: {
            labels: ['Greedy', 'Backward DP', 'Forward DP', '2D Graph Cut'],
            datasets: [{
                data: [18.2, 45.6, 52.1, 88.4],
                backgroundColor: [
                    defaultChartColors.greedy.bg,
                    defaultChartColors.backward_dp.bg,
                    defaultChartColors.forward_dp.bg,
                    defaultChartColors.graph_cut.bg
                ],
                borderColor: [
                    defaultChartColors.greedy.border,
                    defaultChartColors.backward_dp.border,
                    defaultChartColors.forward_dp.border,
                    defaultChartColors.graph_cut.border
                ],
                borderWidth: 1.5,
                borderRadius: 6
            }]
        },
        options: {
            ...commonOptions,
            scales: {
                ...commonOptions.scales,
                y: {
                    ...commonOptions.scales.y,
                    title: { display: true, text: 'Runtime (ms)', color: '#64748b', font: { size: 10 } }
                }
            }
        }
    });

    // Throughput Chart (MPix/s)
    throughputChart = new Chart(ctxThroughput, {
        type: 'bar',
        data: {
            labels: ['Greedy', 'Backward DP', 'Forward DP', '2D Graph Cut'],
            datasets: [{
                data: [28.5, 11.4, 9.9, 5.8],
                backgroundColor: [
                    defaultChartColors.greedy.bg,
                    defaultChartColors.backward_dp.bg,
                    defaultChartColors.forward_dp.bg,
                    defaultChartColors.graph_cut.bg
                ],
                borderColor: [
                    defaultChartColors.greedy.border,
                    defaultChartColors.backward_dp.border,
                    defaultChartColors.forward_dp.border,
                    defaultChartColors.graph_cut.border
                ],
                borderWidth: 1.5,
                borderRadius: 6
            }]
        },
        options: {
            ...commonOptions,
            scales: {
                ...commonOptions.scales,
                y: {
                    ...commonOptions.scales.y,
                    title: { display: true, text: 'Megapixels / sec', color: '#64748b', font: { size: 10 } }
                }
            }
        }
    });
}

function updateBenchmarkCharts(benchmarks) {
    if (!runtimeChart || !throughputChart || !benchmarks) return;

    const labels = [];
    const runtimes = [];
    const throughputs = [];
    const bgColors = [];
    const borderColors = [];

    const labelMap = {
        greedy: 'Greedy O(H)',
        backward_dp: 'Backward DP',
        forward_dp: 'Forward DP',
        graph_cut: '2D Graph Cut'
    };

    const tableBody = document.getElementById('benchmark-table-body');
    if (tableBody) tableBody.innerHTML = '';

    const complexityMap = {
        greedy: 'O(H)',
        backward_dp: 'O(W*H)',
        forward_dp: 'O(W*H)',
        graph_cut: 'Min-Cut Polynomial'
    };

    benchmarks.forEach(b => {
        const id = b.algorithm_id;
        const shortName = labelMap[id] || id;
        labels.push(shortName);
        runtimes.push(b.runtime_ms);
        throughputs.push(b.throughput_mpix_s);

        const colors = defaultChartColors[id] || { bg: 'rgba(6, 182, 212, 0.7)', border: '#06b6d4' };
        bgColors.push(colors.bg);
        borderColors.push(colors.border);

        if (tableBody) {
            const tr = document.createElement('tr');
            tr.className = 'hover:bg-slate-900/40 transition';
            tr.innerHTML = `
                <td class="p-2.5 font-semibold text-slate-200">${b.algorithm_name}</td>
                <td class="p-2.5 text-cyan-400">${complexityMap[id] || 'O(W*H)'}</td>
                <td class="p-2.5 font-bold text-slate-100">${b.runtime_ms.toFixed(1)} ms</td>
                <td class="p-2.5 text-violet-400 font-bold">${b.throughput_mpix_s.toFixed(2)} MPix/s</td>
                <td class="p-2.5 text-slate-400">${b.mean_seam_energy.toFixed(1)}</td>
            `;
            tableBody.appendChild(tr);
        }
    });

    runtimeChart.data.labels = labels;
    runtimeChart.data.datasets[0].data = runtimes;
    runtimeChart.data.datasets[0].backgroundColor = bgColors;
    runtimeChart.data.datasets[0].borderColor = borderColors;
    runtimeChart.update();

    throughputChart.data.labels = labels;
    throughputChart.data.datasets[0].data = throughputs;
    throughputChart.data.datasets[0].backgroundColor = bgColors;
    throughputChart.data.datasets[0].borderColor = borderColors;
    throughputChart.update();
}

window.initCharts = initCharts;
window.updateBenchmarkCharts = updateBenchmarkCharts;
