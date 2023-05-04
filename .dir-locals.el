((c-mode . ((c-file-style . "stroustrup")
            (c-auto-align-backslashes . nil)
            (c-file-offsets
             ;; 'case' labels get a half-indent.  (The only place I've
             ;; ever seen this style is in DOOM, and I don't know of a
             ;; formatter (clang-format, GNU indent, astyle, ...) that
             ;; support it :shrug:).
             (case-label . *)
             (statement-case-intro . *)
             ;; Allow (but don't force) #define'd strings to not be
             ;; indented:
             ;;
             ;;    #define NEWGAME \
             ;;    "you can't start a new game\n"\
             ;;    ...
             (cpp-define-intro
              . ((lambda (langelem)
                   (save-excursion
                     (back-to-indentation)
                     (when (and (eq (char-after) ?\")
                                (bolp))
                       0)))
                 +))
             ;; When wrapping a string to multiple lines in a
             ;; brace-list-entry, indent the following lines so that
             ;; it's easy to tell that it's a continuation of the
             ;; string on the previous line:
             ;;
             ;;    {
             ;;        HKEY_LOCAL_MACHINE,
             ;;        SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
             ;;            "Uninstall\\Ultimate Doom for Windows 95",          <------ this line
             ;;        "UninstallString",
             ;;    },
             (brace-list-entry
              . (lambda (langelem)
                  (save-excursion ;; Why is this (save-excursion) necessary?
                    (let ((abs (c-lineup-string-cont langelem)))
                      (when abs
                        (vector (min (+ (c-langelem-col langelem) c-basic-offset)
                                     (elt abs 0))))))))
             ;; Several things in multi-line parenthesized things
             ;; (if/loop conditions, function arguments, ...).
             (arglist-cont-nonempty
              . (;; Allow (but don't force) lining up complex
                 ;; expressions:
                 ;;
                 ;; There are two flavors of this option:
                 ;;
                 ;;  - line up first-word to first-word:
                 ;;
                 ;;        if (     fl->a.x < 0 || fl->a.x >= f_w
                 ;;              || fl->a.y < 0 || fl->a.y >= f_h
                 ;;              ...
                 ;;
                 ;;  - line up beginning-of-line to paren:
                 ;;
                 ;;        if (     fl->a.x < 0 || fl->a.x >= f_w
                 ;;           || fl->a.y < 0 || fl->a.y >= f_h
                 ;;           ...
                 ;;
                 ;; We support the "normal" lining up inside the paren
                 ;; by falling through to the normal c-lineup-arglist.
                 (lambda (langelem)
                   (save-excursion
                     (back-to-indentation)
                     (when (looking-at (concat c-arithmetic-op-regexp " *"))
                       (let ((opwidth  (- (match-end 0) (match-beginning 0)))
                             (startpos (c-langelem-pos langelem))
                             (endpos   (c-point 'bol))
                             (curcol   (current-column))
                             parencol
                             wordcol)
                         ;; Find the "("
                         ;; This is closely based on
                         ;; cc-align.el:c-lineup-arglist-close-under-paren.
                         (if (memq (c-langelem-sym langelem)
                                   '(arglist-cont-nonempty arglist-close))
                             (goto-char (c-langelem-2nd-pos c-syntactic-element))
                           (beginning-of-line)
                           (c-go-up-list-backward))
                         (unless (save-excursion (c-block-in-arglist-dwim (point)))
                           (let (special-list)
                             (if (and c-special-brace-lists
                                      (setq special-list (c-looking-at-special-brace-list)))
                                 (goto-char (car (car special-list)))))
                           (setq parencol (current-column))
                           (when (re-search-forward "( *[^ ]" (c-point 'eol) t)
                             (backward-char)
                             (setq wordcol (- (current-column) opwidth))))
                         ;; Return.
                         (when (or (eq curcol parencol)
                                   (eq curcol wordcol))
                           (vector curcol))))))
                 ;; Line up strings that are split accross lines.
                 c-lineup-string-cont
                 ;; Otherwise, just do a normal indent.
                 c-lineup-arglist))
             ;; Several things about multi-line assignments:
             (statement-cont
              . (;; When assigning a variable to a struct literal, allow
                 ;; the opening '{' to be on a new line, non-indented:
                 ;;
                 ;;    foo =
                 ;;    {
                 ;;        ...
                 (lambda (langelem)
                   (save-excursion
                     (back-to-indentation)
                     (when (eq (char-after) ?{)
                       0)))
                 ;; Allow (but don't force) lining up complex
                 ;; expressions in assignments:
                 ;;
                 ;;    myvar = foo
                 ;;         && bar
                 (lambda (langelem)
                   (save-excursion
                     (back-to-indentation)
                     (when (looking-at (concat c-arithmetic-op-regexp " *"))
                       (let ((opwidth  (- (match-end 0) (match-beginning 0)))
                             (startpos (c-langelem-pos langelem))
                             (endpos   (c-point 'bol))
                             (curcol   (current-column))
                             aligncol)
                         ;; Find the "=", then skip any whitespace.
                         (goto-char startpos)
                         (when (c-syntactic-re-search-forward (concat c-assignment-op-regexp " *")
                                                              (min endpos (c-point 'eol)) ;; bound
                                                              t ;; noerror
                                                              t ;; paren-level
                                                              t ;; not-inside-token
                                                              )
                           (setq aligncol (- (current-column) opwidth))
                         ;; Check that all lines between the "=" and us also matched this rule.
                         (while (and aligncol (< (point) endpos))
                           (forward-to-indentation)
                           (unless (looking-at c-arithmetic-op-regexp)
                             (setq aligncol nil)))
                         ;; Return.
                         (when (eq curcol aligncol)
                           (vector curcol)))))))
                 ;; Otherwise, just do a normal indent.
                 +))
             ))))
