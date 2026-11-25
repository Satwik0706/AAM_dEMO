import time

def compute_prefix_function(pattern: str):
    """
    Computes the pi prefix function (LPS array) for KMP in O(m) time.
    pi[q] = length of the longest prefix of P that is a proper suffix of P[:q+1]
    """
    m = len(pattern)
    pi = [0] * m
    k = 0
    for q in range(1, m):
        while k > 0 and pattern[k] != pattern[q]:
            k = pi[k - 1]
        if pattern[k] == pattern[q]:
            k += 1
        pi[q] = k
    return pi

def run_kmp(text: str, pattern: str):
    """
    Knuth-Morris-Pratt (KMP) String Matching Algorithm
    Preprocessing Time: O(m)
    Matching Time: O(n)
    Space Complexity: O(m)
    """
    start_time = time.perf_counter()
    n = len(text)
    m = len(pattern)
    
    matches = []
    steps = []
    comparisons = 0
    shifts_using_pi = 0
    
    if m == 0 or n == 0 or m > n:
        return {
            "algorithm": "Knuth-Morris-Pratt (KMP)",
            "matches": matches,
            "total_matches": len(matches),
            "comparisons": comparisons,
            "shifts_using_pi": shifts_using_pi,
            "pi_table": [],
            "execution_time_us": round((time.perf_counter() - start_time) * 1e6, 2),
            "steps": steps,
            "time_complexity": "Preprocessing O(m), Matching O(n)",
            "space_complexity": "O(m)"
        }

    pi = compute_prefix_function(pattern)
    
    # Preprocessing step log
    pi_explanation = [{"index": idx, "char": pattern[idx], "pi_val": val} for idx, val in enumerate(pi)]
    steps.append({
        "step_number": len(steps) + 1,
        "text_index": -1,
        "pattern_index": -1,
        "shift": 0,
        "char_text": "",
        "char_pattern": "",
        "state": "init",
        "pi_table": pi,
        "explanation": f"KMP Preprocessing Complete: Generated pi array {pi}. This avoids backtracking text pointer upon mismatch."
    })

    q = 0  # Number of characters matched
    for i in range(n):
        # Fallback using pi table when mismatch occurs
        while q > 0 and pattern[q] != text[i]:
            comparisons += 1
            old_q = q
            q = pi[q - 1]
            shifts_using_pi += 1
            steps.append({
                "step_number": len(steps) + 1,
                "text_index": i,
                "pattern_index": old_q,
                "shift": i - q,
                "char_text": text[i],
                "char_pattern": pattern[old_q],
                "matched_count": q,
                "state": "pi_fallback",
                "explanation": f"Mismatch at text[{i}]='{text[i]}' and pattern[{old_q}]='{pattern[old_q]}'. SMART SHIFT: pi[{old_q - 1}] = {q}. Resumed matching from pattern index {q} without moving text pointer."
            })
            
        comparisons += 1
        if pattern[q] == text[i]:
            steps.append({
                "step_number": len(steps) + 1,
                "text_index": i,
                "pattern_index": q,
                "shift": i - q,
                "char_text": text[i],
                "char_pattern": pattern[q],
                "matched_count": q + 1,
                "state": "matched_char",
                "explanation": f"Matched: text[{i}]='{text[i]}' == pattern[{q}]='{pattern[q]}'. Characters matched so far: {q + 1}/{m}."
            })
            q += 1
        else:
            steps.append({
                "step_number": len(steps) + 1,
                "text_index": i,
                "pattern_index": q,
                "shift": i,
                "char_text": text[i],
                "char_pattern": pattern[q],
                "matched_count": 0,
                "state": "mismatch",
                "explanation": f"Mismatch at text[{i}]='{text[i]}' and pattern[0]='{pattern[0]}'. Advancing text pointer to index {i + 1}."
            })
            
        if q == m:
            shift = i - m + 1
            matches.append(shift)
            steps.append({
                "step_number": len(steps) + 1,
                "text_index": i,
                "pattern_index": m - 1,
                "shift": shift,
                "char_text": text[i],
                "char_pattern": pattern[m - 1],
                "matched_count": m,
                "state": "found",
                "explanation": f"COMPLETE PATTERN FOUND at shift {shift} (indices {shift}..{i})! Using pi[{m - 1}]={pi[m - 1]} to search for overlapping occurrences."
            })
            q = pi[q - 1]
            shifts_using_pi += 1

    execution_time_us = round((time.perf_counter() - start_time) * 1e6, 2)
    return {
        "algorithm": "Knuth-Morris-Pratt (KMP)",
        "matches": matches,
        "total_matches": len(matches),
        "comparisons": comparisons,
        "shifts_using_pi": shifts_using_pi,
        "pi_table": pi,
        "pi_explanation": pi_explanation,
        "execution_time_us": execution_time_us,
        "steps": steps,
        "time_complexity": "Preprocessing O(m), Matching O(n)",
        "space_complexity": "O(m)"
    }
