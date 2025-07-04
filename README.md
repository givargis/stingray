## Packages

```
$ sudo apt-get update
$ sudo apt-get upgrade
$ sudo apt-get install emacs
$ sudo apt-get install build-essential valgrind
```

## .emacs

```
(defun my-c-mode ()
  (interactive)
  (c-mode)
  (setq version-control 't)
  (c-set-style "linux")
  (setq tab-width 8)
  (setq c-basic-offset 8))
(add-to-list 'auto-mode-alist '("\\.c\\'" . my-c-mode))
(add-to-list 'auto-mode-alist '("\\.h\\'" . my-c-mode))
(add-hook 'before-save-hook 'delete-trailing-whitespace)
```