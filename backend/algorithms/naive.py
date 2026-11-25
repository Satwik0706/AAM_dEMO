import time

def run_naive(text: str, pattern: str):
    """
    Naive String Matching Algorithm
    Slides a window of length m over text of length n.
    Compares characters one-by-one.
    Time Complexity: O((n - m + 1) * m)
    Space Complexity: O(1)
    """
    start_time = time.perf_counter()
    n = len(text)
    m = len(pattern)
    
    matches = []
    steps = []
    comparisons = 0
    
    if m == 0 or n == 0 or m > n:
        return {
            "algorithm": "Naive String Matching",
            "matches": matches,
            "total_matches": len(matches),
            "comparisons": comparisons,
            "execution_time_us": round((time.perf_counter() - start_time) * 1e6, 2),
            "steps": steps,
            "time_complexity": "O((n - m + 1) * m)",
            "space_complexity": "O(1)"
        }

    for s in range(n - m + 1):
        window_matched = True
        step_comparisons = []
        
        for j in range(m):
            comparisons += 1
            text_char = text[s + j]
            pattern_char = pattern[j]
            is_match = (text_char == pattern_char)
            
            step_comparisons.append({
                "pattern_index": j,
                "text_index": s + j,
                "char_text": text_char,
                "char_pattern": pattern_char,
                "matched": is_match
            })
            
            # Step snapshot
            steps.append({
                "step_number": len(steps) + 1,
                "shift": s,
                "pattern_index": j,
                "text_index": s + j,
                "window": text[s:s + m],
                "char_text": text_char,
                "char_pattern": pattern_char,
                "is_char_match": is_match,
                "state": "checking",
                "explanation": f"Comparing text[{s + j}]='{text_char}' with pattern[{j}]='{pattern_char}' (Shift = {s})."
            })
            
            if not is_match:
                window_matched = False
                steps.append({
                    "step_number": len(steps) + 1,
                    "shift": s,
                    "pattern_index": j,
                    "text_index": s + j,
                    "window": text[s:s + m],
                    "char_text": text_char,
                    "char_pattern": pattern_char,
                    "is_char_match": False,
                    "state": "mismatch",
                    "explanation": f"Mismatch detected: '{text_char}' != '{pattern_char}'. Shifting pattern window by +1."
                })
                break
                
        if window_matched:
            matches.append(s)
            steps.append({
                "step_number": len(steps) + 1,
                "shift": s,
                "pattern_index": m - 1,
                "text_index": s + m - 1,
                "window": text[s:s + m],
                "char_text": text[s + m - 1],
                "char_pattern": pattern[m - 1],
                "is_char_match": True,
                "state": "found",
                "explanation": f"COMPLETE MATCH FOUND at shift index {s} (1-based: {s + 1})! All {m} characters matched."
            })

    execution_time_us = round((time.perf_counter() - start_time) * 1e6, 2)
    return {
        "algorithm": "Naive String Matching",
        "matches": matches,
        "total_matches": len(matches),
        "comparisons": comparisons,
        "execution_time_us": execution_time_us,
        "steps": steps,
        "time_complexity": "O((n - m + 1) * m)",
        "space_complexity": "O(1)"
    }
