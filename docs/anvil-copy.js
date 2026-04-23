/* anvil-copy.js — adds a "copy" button to every div.fragment code block.
 *
 * Click strikes the anvil: button flashes rune-green, text reads "STRUCK",
 * and the block copies its source to the clipboard. No deps, degrades
 * gracefully when navigator.clipboard is unavailable (falls back to
 * document.execCommand). Honors prefers-reduced-motion for the flash. */
(function () {
  "use strict";

  var reduceMotion =
    window.matchMedia &&
    window.matchMedia("(prefers-reduced-motion: reduce)").matches;

  function extractText(fragment) {
    // Doxygen code blocks: each .line is one logical line.
    var lines = fragment.querySelectorAll(".line");
    if (lines.length) {
      var out = [];
      for (var i = 0; i < lines.length; i++) {
        // Strip the lineno column (we don't want 1 2 3 in the paste).
        var clone = lines[i].cloneNode(true);
        var lineno = clone.querySelector(".lineno");
        if (lineno) lineno.remove();
        out.push(clone.textContent.replace(/ /g, " ").replace(/\s+$/g, ""));
      }
      return out.join("\n");
    }
    return (fragment.innerText || fragment.textContent || "").replace(
      / /g,
      " ",
    );
  }

  function copy(text) {
    if (navigator.clipboard && navigator.clipboard.writeText) {
      return navigator.clipboard.writeText(text);
    }
    // Fallback
    return new Promise(function (resolve, reject) {
      try {
        var ta = document.createElement("textarea");
        ta.value = text;
        ta.setAttribute("readonly", "");
        ta.style.position = "fixed";
        ta.style.top = "-1000px";
        document.body.appendChild(ta);
        ta.select();
        var ok = document.execCommand("copy");
        document.body.removeChild(ta);
        ok ? resolve() : reject(new Error("execCommand copy failed"));
      } catch (e) {
        reject(e);
      }
    });
  }

  function strike(btn, ok) {
    var defaultLabel = "⚒ copy"; // hammer + copy
    if (ok) {
      btn.textContent = "✓ copied";
      if (!reduceMotion) btn.classList.add("coa-struck");
    } else {
      btn.textContent = "✗ failed";
    }
    setTimeout(function () {
      btn.textContent = defaultLabel;
      btn.classList.remove("coa-struck");
    }, 1200);
  }

  function attach(fragment) {
    if (fragment.querySelector(":scope > .coa-copy-btn")) return;
    // Fragment must be the positioning context
    var cs = window.getComputedStyle(fragment);
    if (cs.position === "static") fragment.style.position = "relative";

    var btn = document.createElement("button");
    btn.type = "button";
    btn.className = "coa-copy-btn";
    btn.setAttribute("aria-label", "Copy code to clipboard");
    btn.textContent = "⚒ copy";

    btn.addEventListener("click", function (ev) {
      ev.preventDefault();
      var text = extractText(fragment);
      copy(text).then(
        function () {
          strike(btn, true);
        },
        function () {
          strike(btn, false);
        },
      );
    });

    fragment.appendChild(btn);
  }

  function init() {
    var blocks = document.querySelectorAll("div.fragment");
    for (var i = 0; i < blocks.length; i++) attach(blocks[i]);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
