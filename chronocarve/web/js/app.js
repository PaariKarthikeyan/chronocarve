// ChronoCarve: Content-Aware Image Reframing Engine Frontend Logic
// Multi-paradigm seam carving, dual viewports, seams visualization & benchmarks

document.addEventListener('DOMContentLoaded', () => {
    // State
    let currentImageMeta = {
        filename: 'sample_mountain_lake.jpg',
        url: '/uploads/sample_mountain_lake.jpg',
        width: 3840,
        height: 2160,
        aspect_ratio: 1.778
    };

    let selectedRatio = '9:16';
    let selectedQuality = '4k';
    let currentTargetWidth = 1214;

    // DOM Elements - Source Selection
    const sampleBtns = document.querySelectorAll('.sample-btn');
    const dropZone = document.getElementById('drop-zone');
    const fileInput = document.getElementById('image-file-input');
    const imageSpecs = document.getElementById('image-specs');
    const metaRes = document.getElementById('meta-res');
    const metaAspect = document.getElementById('meta-aspect');

    // DOM Elements - Target Aspect Ratio & Sliders
    const aspectBtns = document.querySelectorAll('.aspect-btn');
    const qualityBtns = document.querySelectorAll('.quality-btn');
    const targetResVal = document.getElementById('target-res-val');
    const targetSeamsCount = document.getElementById('target-seams-count');
    const customWSlider = document.getElementById('custom-w-slider');
    const customWVal = document.getElementById('custom-w-val');

    // DOM Elements - Action Buttons & Progress
    const runBtn = document.getElementById('run-btn');
    const benchmarkAllBtn = document.getElementById('benchmark-all-btn');
    const progressContainer = document.getElementById('progress-container');
    const progressBar = document.getElementById('progress-bar');
    const progressMsg = document.getElementById('progress-msg');
    const progressPct = document.getElementById('progress-pct');

    // DOM Elements - Viewport Tabs & Images
    const tabDual = document.getElementById('tab-dual');
    const tabSeams = document.getElementById('tab-seams');
    const tabHeatmap = document.getElementById('tab-heatmap');

    const viewDual = document.getElementById('view-dual');
    const viewSeams = document.getElementById('view-seams');
    const viewHeatmap = document.getElementById('view-heatmap');

    const imgOriginal = document.getElementById('img-original');
    const imgRetargeted = document.getElementById('img-retargeted');
    const imgSeams = document.getElementById('img-seams');
    const imgHeatmap = document.getElementById('img-heatmap');
    const retargetPlaceholder = document.getElementById('retarget-placeholder');

    const origSizeLabel = document.getElementById('orig-size-label');
    const retargetSizeLabel = document.getElementById('retarget-size-label');

    // DOM Elements - Downloads
    const downloadHeaderBtn = document.getElementById('download-header-btn');
    const downloadCard = document.getElementById('download-card');
    const directDownloadBtn = document.getElementById('direct-download-btn');
    const openTabBtn = document.getElementById('open-tab-btn');
    const downloadBadgeRes = document.getElementById('download-badge-res');

    // DOM Elements - Metrics & Benchmark
    const statRuntime = document.getElementById('stat-runtime');
    const statThroughput = document.getElementById('stat-throughput');
    const statSeams = document.getElementById('stat-seams');
    const statEnergy = document.getElementById('stat-energy');
    const benchmarkSection = document.getElementById('benchmark-section');

    // Initialize Chart.js
    if (window.initCharts) {
        window.initCharts();
    }

    // --- Tab Switching Logic ---
    function setActiveTab(activeTab, activeView) {
        [tabDual, tabSeams, tabHeatmap].forEach(t => {
            t.classList.remove('active', 'border-cyan-800/60', 'bg-cyan-950/80', 'text-cyan-300');
            t.classList.add('border-glassBorder', 'bg-slate-900/40', 'text-slate-400');
        });
        activeTab.classList.add('active', 'border-cyan-800/60', 'bg-cyan-950/80', 'text-cyan-300');
        activeTab.classList.remove('border-glassBorder', 'bg-slate-900/40', 'text-slate-400');

        viewDual.classList.add('hidden');
        viewSeams.classList.add('hidden');
        viewHeatmap.classList.add('hidden');
        activeView.classList.remove('hidden');
    }

    tabDual.addEventListener('click', () => setActiveTab(tabDual, viewDual));
    tabSeams.addEventListener('click', () => setActiveTab(tabSeams, viewSeams));
    tabHeatmap.addEventListener('click', () => setActiveTab(tabHeatmap, viewHeatmap));

    // --- Target Aspect Ratio & Quality Calculation ---
    function updateTargetWidthCalculation() {
        if (!currentImageMeta) return;

        let targetH = currentImageMeta.height;
        if (selectedQuality === '4k') {
            targetH = 2160;
        } else if (selectedQuality === '1080p') {
            targetH = 1080;
        } else {
            targetH = currentImageMeta.height;
        }

        const qualityPrefix = selectedQuality === '4k' ? '4K UHD ' : (selectedQuality === '1080p' ? '1080p FHD ' : 'Native ');

        if (selectedRatio === '9:16') {
            const ideal = Math.round(targetH * 9.0 / 16.0);
            currentTargetWidth = Math.min(3840, ideal % 2 === 0 ? ideal : ideal - 1);
            targetResVal.textContent = `${currentTargetWidth} x ${targetH} (${qualityPrefix}Story / Portrait)`;
        } else if (selectedRatio === '1:1') {
            const ideal = targetH;
            currentTargetWidth = ideal % 2 === 0 ? ideal : ideal - 1;
            targetResVal.textContent = `${currentTargetWidth} x ${targetH} (${qualityPrefix}Square Feed)`;
        } else if (selectedRatio === '4:5') {
            const ideal = Math.round(targetH * 4.0 / 5.0);
            currentTargetWidth = ideal % 2 === 0 ? ideal : ideal - 1;
            targetResVal.textContent = `${currentTargetWidth} x ${targetH} (${qualityPrefix}Portrait Post)`;
        } else if (selectedRatio === 'custom') {
            currentTargetWidth = parseInt(customWSlider.value, 10);
            targetResVal.textContent = `${currentTargetWidth} x ${targetH} (${qualityPrefix}Custom)`;
        }

        // Relative seams removed from original resolution
        const scaleToSource = currentImageMeta.height / targetH;
        const sourceWidthTarget = Math.round(currentTargetWidth * scaleToSource);
        const seams = Math.max(0, currentImageMeta.width - sourceWidthTarget);
        targetSeamsCount.textContent = `${seams} px`;

        customWSlider.value = currentTargetWidth;
        customWVal.textContent = `${currentTargetWidth} px`;
    }

    aspectBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            aspectBtns.forEach(b => {
                b.classList.remove('active', 'border-cyan-500', 'bg-cyan-950/60', 'text-cyan-300');
                b.classList.add('border-glassBorder', 'bg-slate-900/40', 'text-slate-300');
            });
            btn.classList.add('active', 'border-cyan-500', 'bg-cyan-950/60', 'text-cyan-300');
            btn.classList.remove('border-glassBorder', 'bg-slate-900/40', 'text-slate-300');
            selectedRatio = btn.getAttribute('data-ratio');
            updateTargetWidthCalculation();
        });
    });

    qualityBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            qualityBtns.forEach(b => {
                b.classList.remove('active', 'border-cyan-500', 'bg-cyan-950/60', 'text-cyan-300');
                b.classList.add('border-glassBorder', 'bg-slate-900/40', 'text-slate-300');
            });
            btn.classList.add('active', 'border-cyan-500', 'bg-cyan-950/60', 'text-cyan-300');
            btn.classList.remove('border-glassBorder', 'bg-slate-900/40', 'text-slate-300');
            selectedQuality = btn.getAttribute('data-quality');
            updateTargetWidthCalculation();
        });
    });

    customWSlider.addEventListener('input', (e) => {
        selectedRatio = 'custom';
        aspectBtns.forEach(b => {
            b.classList.remove('active', 'border-cyan-500', 'bg-cyan-950/60', 'text-cyan-300');
            b.classList.add('border-glassBorder', 'bg-slate-900/40', 'text-slate-300');
        });
        currentTargetWidth = parseInt(e.target.value, 10);
        customWVal.textContent = `${currentTargetWidth} px`;
        let targetH = currentImageMeta.height;
        if (selectedQuality === '4k') targetH = 2160;
        else if (selectedQuality === '1080p') targetH = 1080;
        const qualityPrefix = selectedQuality === '4k' ? '4K UHD ' : (selectedQuality === '1080p' ? '1080p ' : '');
        targetResVal.textContent = `${currentTargetWidth} x ${targetH} (${qualityPrefix}Custom)`;
        const scaleToSource = currentImageMeta.height / targetH;
        const sourceWidthTarget = Math.round(currentTargetWidth * scaleToSource);
        const seams = Math.max(0, currentImageMeta.width - sourceWidthTarget);
        targetSeamsCount.textContent = `${seams} px`;
    });

    // --- Image Setup & Metadata Rendering ---
    function setLoadedImage(meta) {
        currentImageMeta = meta;

        const is4k = (meta.width >= 3840 || meta.height >= 2160);
        imageSpecs.textContent = `${meta.width}x${meta.height} (${is4k ? '4K Ultra-HD' : 'HD'})`;
        metaRes.textContent = `${meta.width} x ${meta.height}`;
        metaAspect.textContent = `${meta.aspect_ratio.toFixed(2)}:1`;

        imgOriginal.src = meta.url;
        origSizeLabel.textContent = `${meta.width} x ${meta.height} (${is4k ? '4K UHD' : 'HD'})`;

        customWSlider.min = 100;
        customWSlider.max = Math.max(3840, meta.width);

        updateTargetWidthCalculation();

        // Reset outputs
        imgRetargeted.classList.add('hidden');
        retargetPlaceholder.classList.remove('hidden');
        retargetSizeLabel.textContent = 'Ready';
        downloadHeaderBtn.classList.add('hidden');
        downloadCard.classList.add('hidden');
    }

    // --- Sample Landscape Selection ---
    sampleBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            sampleBtns.forEach(b => {
                b.classList.remove('active', 'border-cyan-500', 'bg-cyan-950/60');
                b.classList.add('border-glassBorder', 'bg-slate-900/40');
            });
            btn.classList.add('active', 'border-cyan-500', 'bg-cyan-950/60');
            btn.classList.remove('border-glassBorder', 'bg-slate-900/40');

            const filename = btn.getAttribute('data-sample');
            const img = new Image();
            img.onload = () => {
                const w = img.naturalWidth || 3840;
                const h = img.naturalHeight || 2160;
                setLoadedImage({
                    filename: filename,
                    url: `/uploads/${filename}`,
                    width: w,
                    height: h,
                    aspect_ratio: w / h
                });
            };
            img.src = `/uploads/${filename}`;
        });
    });

    // --- File Drag & Drop / Upload ---
    dropZone.addEventListener('click', () => fileInput.click());

    dropZone.addEventListener('dragover', (e) => {
        e.preventDefault();
        dropZone.classList.add('border-cyan-400', 'bg-cyan-950/20');
    });

    dropZone.addEventListener('dragleave', () => {
        dropZone.classList.remove('border-cyan-400', 'bg-cyan-950/20');
    });

    dropZone.addEventListener('drop', (e) => {
        e.preventDefault();
        dropZone.classList.remove('border-cyan-400', 'bg-cyan-950/20');
        if (e.dataTransfer.files.length > 0) {
            handleFileUpload(e.dataTransfer.files[0]);
        }
    });

    fileInput.addEventListener('change', (e) => {
        if (e.target.files.length > 0) {
            handleFileUpload(e.target.files[0]);
        }
    });

    async function handleFileUpload(file) {
        const formData = new FormData();
        formData.append('image', file);

        progressContainer.classList.remove('hidden');
        progressMsg.textContent = 'Uploading image...';
        progressBar.style.width = '30%';
        progressPct.textContent = '30%';

        try {
            const resp = await fetch('/api/upload', {
                method: 'POST',
                body: formData
            });
            const data = await resp.json();
            if (data.success) {
                progressBar.style.width = '100%';
                progressPct.textContent = '100%';
                setTimeout(() => progressContainer.classList.add('hidden'), 400);

                sampleBtns.forEach(b => {
                    b.classList.remove('active', 'border-cyan-500', 'bg-cyan-950/60');
                    b.classList.add('border-glassBorder', 'bg-slate-900/40');
                });
                setLoadedImage(data);
            } else {
                alert('Upload error: ' + (data.error || 'Unknown error'));
                progressContainer.classList.add('hidden');
            }
        } catch (err) {
            console.error('Upload failed:', err);
            alert('Upload failed: server error.');
            progressContainer.classList.add('hidden');
        }
    }

    // --- Run Selected Algorithm ---
    runBtn.addEventListener('click', async () => {
        if (!currentImageMeta) {
            alert('Please select or upload an image first.');
            return;
        }

        const selectedAlgo = document.querySelector('input[name="algorithm"]:checked').value;
        const targetWidth = currentTargetWidth;

        const payload = {
            filename: currentImageMeta.filename,
            algorithm: selectedAlgo,
            quality: selectedQuality,
            ratio: selectedRatio,
            target_width: targetWidth,
            alpha: 1.0,
            highlight_seams: true
        };

        runBtn.disabled = true;
        runBtn.classList.add('opacity-70');
        progressContainer.classList.remove('hidden');
        progressBar.style.width = '45%';
        progressPct.textContent = 'Carving...';
        progressMsg.textContent = 'Computing content-aware seams...';

        try {
            const resp = await fetch('/api/process', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            const data = await resp.json();

            if (data.success) {
                progressBar.style.width = '100%';
                progressPct.textContent = 'Done!';
                progressMsg.textContent = 'Reframing Complete!';
                setTimeout(() => progressContainer.classList.add('hidden'), 500);

                // Display Reframed Result
                imgRetargeted.src = data.reframed_image_url + '?t=' + Date.now();
                imgRetargeted.classList.remove('hidden');
                retargetPlaceholder.classList.add('hidden');

                // Display Seam Map Overlay
                imgSeams.src = data.seam_overlay_url + '?t=' + Date.now();

                // Display Energy Heatmap
                imgHeatmap.src = data.energy_heatmap_url + '?t=' + Date.now();

                // Update labels and download links
                const m = data.metrics;
                retargetSizeLabel.textContent = `${m.target_width} x ${m.target_height} (${m.is_4k ? '4K Ultra-HD' : 'HD'})`;
                if (downloadBadgeRes) {
                    downloadBadgeRes.textContent = `${m.target_width}x${m.target_height} (${m.is_4k ? '4K UHD' : 'HD'} Story)`;
                }

                downloadHeaderBtn.href = data.download_url;
                downloadHeaderBtn.classList.remove('hidden');

                downloadCard.classList.remove('hidden');
                directDownloadBtn.href = data.download_url;
                directDownloadBtn.setAttribute('download', `chronocarve_${m.target_width}x${m.target_height}.jpg`);
                openTabBtn.href = data.reframed_image_url;

                // Update Metrics Cards
                statRuntime.textContent = `${m.runtime_ms.toFixed(1)} ms`;
                statThroughput.textContent = `${m.throughput_mpix_s.toFixed(2)} MPix/s`;
                statSeams.textContent = `${m.seams_removed} px`;
                statEnergy.textContent = `${Math.round(m.total_energy_removed).toLocaleString()}`;
            } else {
                alert('Reframing error: ' + (data.error || 'Server error'));
                progressContainer.classList.add('hidden');
            }
        } catch (err) {
            console.error('Reframing process failed:', err);
            alert('Reframing failed due to network error.');
            progressContainer.classList.add('hidden');
        } finally {
            runBtn.disabled = false;
            runBtn.classList.remove('opacity-70');
        }
    });

    // --- Benchmark Suite ---
    benchmarkAllBtn.addEventListener('click', async () => {
        if (!currentImageMeta) return;

        benchmarkAllBtn.disabled = true;
        benchmarkAllBtn.classList.add('opacity-70');
        progressContainer.classList.remove('hidden');
        progressBar.style.width = '30%';
        progressPct.textContent = 'Benchmarking...';
        progressMsg.textContent = 'Running 4-way comparative benchmark suite...';

        try {
            const resp = await fetch('/api/benchmark-suite', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    filename: currentImageMeta.filename
                })
            });
            const data = await resp.json();

            if (data.success && data.benchmarks) {
                progressBar.style.width = '100%';
                progressPct.textContent = 'Complete!';
                progressMsg.textContent = 'Benchmark Finished!';
                setTimeout(() => progressContainer.classList.add('hidden'), 500);

                benchmarkSection.classList.remove('hidden');
                if (window.updateBenchmarkCharts) {
                    window.updateBenchmarkCharts(data.benchmarks);
                }
                benchmarkSection.scrollIntoView({ behavior: 'smooth' });
            } else {
                alert('Benchmark failed: ' + (data.error || 'Unknown error'));
                progressContainer.classList.add('hidden');
            }
        } catch (err) {
            console.error('Benchmark error:', err);
            alert('Benchmark failed: server error.');
            progressContainer.classList.add('hidden');
        } finally {
            benchmarkAllBtn.disabled = false;
            benchmarkAllBtn.classList.remove('opacity-70');
        }
    });

    // Check server status & initialize default sample image
    fetch('/api/sample-images')
        .then(r => r.json())
        .then(data => {
            if (data.success && data.samples && data.samples.length > 0) {
                setLoadedImage(data.samples[0]);
            }
        })
        .catch(() => {
            const badge = document.getElementById('server-status-badge');
            if (badge) badge.textContent = 'Backend Standby';
        });
});
