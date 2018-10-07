;; Sample .dir-locals.el

((nil . ((eval . (set (make-local-variable 'my-project-path)
                      (expand-file-name
                       (locate-dominating-file default-directory
                                               ".dir-locals.el"))))))
 (c++-mode . ((eval . (setq company-clang-arguments
                            (list (concat "-I" my-project-path)
                                  "-I/opt/VTK/8.1.1/include/vtk-8.1"
                                  "-I/opt/mpich/3.2.1/include"
                                  "-std=gnu++11"
                                  (concat "-I" my-project-path "build"))))
              (c-basic-offset . 2)
              (indent-tabs-mode . nil)))
 (nil . ((c-file-style . "linux"))))
