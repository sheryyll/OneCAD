#include "sketch/SnapManager.h"
#include "sketch/Sketch.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace onecad::core::sketch;

namespace {

struct TestResult {
    bool pass = false;
    std::string expected;
    std::string got;
};

bool approx(double a, double b, double tol = 1e-6) {
    return std::abs(a - b) <= tol;
}

Sketch createTestSketch() {
    Sketch sketch;

    sketch.addPoint(5.0, 5.0);

    EntityID origin = sketch.addPoint(0.0, 0.0);
    EntityID hEnd = sketch.addPoint(10.0, 0.0);
    EntityID vEnd = sketch.addPoint(0.0, 10.0);
    sketch.addLine(origin, hEnd);
    sketch.addLine(origin, vEnd);

    EntityID circleCenter = sketch.addPoint(20.0, 20.0);
    sketch.addCircle(circleCenter, 5.0);

    EntityID arcCenter = sketch.addPoint(40.0, 40.0);
    sketch.addArc(arcCenter, 3.0, 0.0, M_PI * 0.5);

    return sketch;
}

SnapManager createSnapManagerFor(const std::vector<SnapType>& enabledTypes) {
    SnapManager manager;
    manager.setAllSnapsEnabled(false);
    manager.setEnabled(true);
    for (SnapType type : enabledTypes) {
        manager.setSnapEnabled(type, true);
        if (type == SnapType::Grid) {
            manager.setGridSnapEnabled(true);
        }
    }
    return manager;
}

TestResult expectSnap(SnapResult result, SnapType type) {
    if (!result.snapped) {
        return {false, "snapped", "not snapped"};
    }
    if (result.type != type) {
        return {
            false,
            std::to_string(static_cast<int>(type)),
            std::to_string(static_cast<int>(result.type))
        };
    }
    return {true, "", ""};
}

TestResult testVertexSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Vertex});
    SnapResult result = manager.findBestSnap({5.2, 5.1}, sketch);
    TestResult check = expectSnap(result, SnapType::Vertex);
    if (!check.pass) {
        return check;
    }
    if (!approx(result.position.x, 5.0) || !approx(result.position.y, 5.0)) {
        return {false, "(5,5)", "(" + std::to_string(result.position.x) + "," + std::to_string(result.position.y) + ")"};
    }
    return {true, "", ""};
}

TestResult testEndpointSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Endpoint});
    SnapResult result = manager.findBestSnap({10.3, 0.2}, sketch);
    return expectSnap(result, SnapType::Endpoint);
}

TestResult testMidpointSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Midpoint});
    SnapResult result = manager.findBestSnap({5.2, 0.1}, sketch);
    return expectSnap(result, SnapType::Midpoint);
}

TestResult testCenterSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Center});
    SnapResult result = manager.findBestSnap({20.3, 20.2}, sketch);
    return expectSnap(result, SnapType::Center);
}

TestResult testQuadrantSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Quadrant});
    SnapResult result = manager.findBestSnap({25.1, 20.2}, sketch);
    return expectSnap(result, SnapType::Quadrant);
}

TestResult testIntersectionSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Intersection});
    SnapResult result = manager.findBestSnap({0.3, 0.2}, sketch);
    return expectSnap(result, SnapType::Intersection);
}

TestResult testOnCurveSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::OnCurve});
    SnapResult result = manager.findBestSnap({20.2, 15.6}, sketch);
    return expectSnap(result, SnapType::OnCurve);
}

TestResult testGridSnap() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Grid});
    manager.setGridSize(1.0);
    SnapResult result = manager.findBestSnap({100.2, 100.2}, sketch);
    return expectSnap(result, SnapType::Grid);
}

TestResult testPriorityOrder() {
    Sketch sketch = createTestSketch();
    SnapManager manager = createSnapManagerFor({SnapType::Vertex, SnapType::Endpoint});
    SnapResult result = manager.findBestSnap({0.1, 0.1}, sketch);
    return expectSnap(result, SnapType::Vertex);
}

bool shouldSkipInLegacy(const std::string& testName) {
    static const std::vector<std::string> blocked = {
        "perpendicular",
        "tangent",
        "angular",
        "horizontal_alignment",
        "vertical_alignment",
        "extension",
        "spatial_hash",
        "ellipse"
    };

    for (const std::string& token : blocked) {
        if (testName.find(token) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void runBenchmark() {
    Sketch sketch;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-500.0, 500.0);

    for (int i = 0; i < 1000; ++i) {
        sketch.addPoint(dist(rng), dist(rng));
    }

    SnapManager manager;
    std::vector<double> queryMicros;
    queryMicros.reserve(100);

    for (int i = 0; i < 100; ++i) {
        Vec2d cursor{dist(rng), dist(rng)};
        auto t0 = std::chrono::steady_clock::now();
        (void)manager.findBestSnap(cursor, sketch);
        auto t1 = std::chrono::steady_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        queryMicros.push_back(us);
    }

    std::sort(queryMicros.begin(), queryMicros.end());
    size_t p95Index = static_cast<size_t>(std::ceil(0.95 * static_cast<double>(queryMicros.size()))) - 1;
    if (p95Index >= queryMicros.size()) {
        p95Index = queryMicros.size() - 1;
    }

    std::cout << "Benchmark: p95 query time " << queryMicros[p95Index] << " us" << std::endl;
}

} // namespace

int main(int argc, char** argv) {
    bool legacyOnly = false;
    bool runBench = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--legacy") {
            legacyOnly = true;
        } else if (arg == "--benchmark") {
            runBench = true;
        }
    }

    const std::vector<std::pair<std::string, std::function<TestResult()>>> tests = {
        {"test_vertex_snap", testVertexSnap},
        {"test_endpoint_snap", testEndpointSnap},
        {"test_midpoint_snap", testMidpointSnap},
        {"test_center_snap", testCenterSnap},
        {"test_quadrant_snap", testQuadrantSnap},
        {"test_intersection_snap", testIntersectionSnap},
        {"test_oncurve_snap", testOnCurveSnap},
        {"test_grid_snap", testGridSnap},
        {"test_priority_order", testPriorityOrder}
    };

    int passed = 0;
    int total = 0;

    for (const auto& [name, fn] : tests) {
        if (legacyOnly && shouldSkipInLegacy(name)) {
            continue;
        }

        ++total;
        TestResult r = fn();
        if (r.pass) {
            ++passed;
            std::cout << "PASS: " << name << std::endl;
        } else {
            std::cout << "FAIL: " << name
                      << " (expected " << r.expected
                      << ", got " << r.got << ")" << std::endl;
        }
    }

    std::cout << passed << "/" << total << " tests passed" << std::endl;

    if (runBench) {
        runBenchmark();
    }

    return passed == total ? 0 : 1;
}
