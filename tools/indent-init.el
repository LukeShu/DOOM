(require 'package)
(setq package-user-dir (concat (file-name-directory load-file-name) "elpa"))
(if (package-installed-p 'editorconfig)
    (package-activate 'editorconfig)
  (package-install 'editorconfig t))
(require 'editorconfig)
(editorconfig-mode 1)

(setq make-backup-files nil
      safe-local-variable-values (mapcan #'cdr (read
                                                (with-temp-buffer
                                                  (insert-file-contents ".dir-locals.el")
                                                  (buffer-string)))))
