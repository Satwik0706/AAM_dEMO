import time

def run_rabin_karp(text: str, pattern: str, d: int = 256, q: int = 101):
    """
    Rabin-Karp Algorithm using Polynomial Rolling Hash
    Time Complexity: Average O(n + m), Worst-case O((n - m + 1) * m)
    Space Complexity: O(1)
    """
    start_time = time.perf_counter()
    n = len(text)
    m = len(pattern)
    
    matches = []
    steps = []
    comparisons = 0
    hash_computations = 0
    spurious_hits = 0
    
    if m == 0 or n == 0 or m > n:
        return {
            "algorithm": "Rabin-Karp Algorithm",
            "matches": matches,
            "total_matches": len(matches),
            "comparisons": comparisons,
            "hash_computations": hash_computations,
            "spurious_hits": spurious_hits,
            "execution_time_us": round((time.perf_counter() - start_time) * 1e6, 2),
            "steps": steps,
            "pattern_hash": 0,
            "prime_modulus": q,
            "radix_d": d,
            "time_complexity": "Average O(n + m), Worst O(n * m)",
            "space_complexity": "O(1)"
        }

    # h = d^(m-1) % q
    h = pow(d, m - 1, q)
    
    # Compute initial hashes for pattern and first window
    p_hash = 0
    t_hash = 0
    for i in range(m):
        p_hash = (d * p_hash + ord(pattern[i])) % q
        t_hash = (d * t_hash + ord(text[i])) % q
        hash_computations += 2
        
    steps.append({
        "step_number": len(steps) + 1,
        "shift": 0,
        "window": text[0:m],
        "pattern_hash": p_hash,
        "window_hash": t_hash,
        "hash_match": (p_hash == t_hash),
        "state": "init",
        "explanation": f"Precomputed Pattern Hash = {p_hash} | Initial Window (shift 0) '{text[0:m]}' Hash = {t_hash} (Modulus q={q}, Base d={d})."
    })

    # Slide the pattern over the text
    for s in range(n - m + 1):
        window_str = text[s:s + m]
        hash_match = (p_hash == t_hash)
        
        steps.append({
            "step_number": len(steps) + 1,
            "shift": s,
            "window": window_str,
            "pattern_hash": p_hash,
            "window_hash": t_hash,
            "hash_match": hash_match,
            "state": "hash_check",
            "explanation": f"Shift {s}: Comparing hashes -> Window Hash [{t_hash}] vs Pattern Hash [{p_hash}]. " + 
                           ("Hashes match! Verifying characters..." if hash_match else "Hashes differ! Skipping character comparisons in O(1).")
        })
        
        if hash_match:
            # Check characters one by one
            char_mismatch = False
            for j in range(m):
                comparisons += 1
                if text[s + j] != pattern[j]:
                    char_mismatch = True
                    spurious_hits += 1
                    steps.append({
                        "step_number": len(steps) + 1,
                        "shift": s,
                        "window": window_str,
                        "pattern_hash": p_hash,
                        "window_hash": t_hash,
                        "hash_match": True,
                        "state": "spurious_hit",
                        "explanation": f"SPURIOUS HIT / HASH COLLISION at shift {s}! Hash matched ({t_hash}) but text[{s + j}]='{text[s + j]}' != pattern[{j}]='{pattern[j]}'."
                    })
                    break
            
            if not char_mismatch:
                matches.append(s)
                steps.append({
                    "step_number": len(steps) + 1,
                    "shift": s,
                    "window": window_str,
                    "pattern_hash": p_hash,
                    "window_hash": t_hash,
                    "hash_match": True,
                    "state": "found",
                    "explanation": f"FULL MATCH VERIFIED at shift {s}! Hash matched ({t_hash}) and all {m} characters confirmed identical."
                })
        
        # Calculate hash value for next window in O(1)
        if s < n - m:
            old_char = text[s]
            new_char = text[s + m]
            # t_{s+1} = (d * (t_s - text[s] * h) + text[s + m]) % q
            prev_t_hash = t_hash
            t_hash = (d * (t_hash - ord(old_char) * h) + ord(new_char)) % q
            # Ensure non-negative modulo in python/c++
            if t_hash < 0:
                t_hash += q
            hash_computations += 1
            
            steps.append({
                "step_number": len(steps) + 1,
                "shift": s + 1,
                "window": text[s + 1:s + 1 + m],
                "pattern_hash": p_hash,
                "window_hash": t_hash,
                "hash_match": False,
                "state": "rolling_hash",
                "explanation": f"Rolling hash computed in O(1): Removed '{old_char}' (ord {ord(old_char)}), added '{new_char}' (ord {ord(new_char)}). New Window Hash = {t_hash} (was {prev_t_hash})."
            })

    execution_time_us = round((time.perf_counter() - start_time) * 1e6, 2)
    return {
        "algorithm": "Rabin-Karp Algorithm",
        "matches": matches,
        "total_matches": len(matches),
        "comparisons": comparisons,
        "hash_computations": hash_computations,
        "spurious_hits": spurious_hits,
        "pattern_hash": p_hash,
        "prime_modulus": q,
        "radix_d": d,
        "execution_time_us": execution_time_us,
        "steps": steps,
        "time_complexity": "Average O(n + m), Worst O(n * m)",
        "space_complexity": "O(1)"
    }
