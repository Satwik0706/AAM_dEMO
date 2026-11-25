from flask import Flask, request, jsonify, render_template, send_from_directory
from flask_cors import CORS
import os
import sys

# Ensure current and parent dirs are on path
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

from algorithms.naive import run_naive
from algorithms.rabin_karp import run_rabin_karp
from algorithms.finite_automata import run_finite_automata
from algorithms.kmp import run_kmp

template_folder = os.path.abspath(os.path.join(current_dir, "..", "frontend", "templates"))
static_folder = os.path.abspath(os.path.join(current_dir, "..", "frontend", "static"))

app = Flask(__name__, template_folder=template_folder, static_folder=static_folder)
CORS(app)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/api/match", methods=["POST"])
def match_string():
    data = request.get_json(force=True, silent=True) or {}
    text = data.get("text", "")
    pattern = data.get("pattern", "")
    algo = data.get("algorithm", "all").lower()
    
    if not text or not pattern:
        return jsonify({"error": "Both 'text' and 'pattern' parameters are required."}), 400
        
    response = {}
    
    if algo in ["all", "naive"]:
        response["naive"] = run_naive(text, pattern)
        
    if algo in ["all", "rabin_karp", "rabin-karp"]:
        q = int(data.get("prime_q", 101))
        response["rabin_karp"] = run_rabin_karp(text, pattern, q=q)
        
    if algo in ["all", "finite_automata", "automata"]:
        response["finite_automata"] = run_finite_automata(text, pattern)
        
    if algo in ["all", "kmp"]:
        response["kmp"] = run_kmp(text, pattern)
        
    # Generate benchmark summary comparison
    if algo == "all":
        comparison = [
            {
                "name": "Naive String Matching",
                "key": "naive",
                "matches": len(response["naive"]["matches"]),
                "comparisons": response["naive"]["comparisons"],
                "time_us": response["naive"]["execution_time_us"],
                "time_complexity": "O((n - m + 1) * m)",
                "space_complexity": "O(1)",
                "best_for": "Small alphabets and short patterns; simple implementation"
            },
            {
                "name": "Rabin-Karp Algorithm",
                "key": "rabin_karp",
                "matches": len(response["rabin_karp"]["matches"]),
                "comparisons": response["rabin_karp"]["comparisons"],
                "hash_computations": response["rabin_karp"]["hash_computations"],
                "spurious_hits": response["rabin_karp"]["spurious_hits"],
                "time_us": response["rabin_karp"]["execution_time_us"],
                "time_complexity": "Average O(n + m), Worst O(n * m)",
                "space_complexity": "O(1)",
                "best_for": "Multiple pattern search, plagiarism detection"
            },
            {
                "name": "Finite Automata Matcher",
                "key": "finite_automata",
                "matches": len(response["finite_automata"]["matches"]),
                "state_transitions": response["finite_automata"]["state_transitions"],
                "time_us": response["finite_automata"]["execution_time_us"],
                "time_complexity": "Matching O(n), Preprocessing O(m * |Sigma|)",
                "space_complexity": "O(m * |Sigma|)",
                "best_for": "Streaming data, network intrusion detection"
            },
            {
                "name": "Knuth-Morris-Pratt (KMP)",
                "key": "kmp",
                "matches": len(response["kmp"]["matches"]),
                "comparisons": response["kmp"]["comparisons"],
                "time_us": response["kmp"]["execution_time_us"],
                "time_complexity": "Preprocessing O(m), Matching O(n)",
                "space_complexity": "O(m)",
                "best_for": "Worst-case guarantee, repeated substring patterns"
            }
        ]
        response["comparison"] = comparison
        
    response["meta"] = {
        "text_length": len(text),
        "pattern_length": len(pattern),
        "text": text,
        "pattern": pattern
    }
    
    return jsonify(response)

@app.route("/api/health")
def health():
    return jsonify({"status": "running", "engine": "Python 3.13 Flask String Matching Suite"})

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    print(f"\n==================================================================")
    print(f" String Matching Interactive Platform started on http://localhost:{port}")
    print(f" Supported: Naive | Rabin-Karp | Finite Automata | KMP")
    print(f"==================================================================\n")
    app.run(host="0.0.0.0", port=port, debug=False)
