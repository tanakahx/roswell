(ros:include "util-install-quicklisp")
(ros:quicklisp :environment nil)
(defpackage :roswell.help.install
  (:use :cl :roswell.util :roswell.install))
(in-package :roswell.help.install)

(defun install (argv)
  (if (not (second argv))
      (let ((s *error-output*)
            (cmd (pathname-name (opt "wargv0"))))
        (format s "Usage:~%~%   ~A install impl [options]~%or~%" cmd)
        (format s "   ~A install repository [repository... ] ~%~%" cmd)
        (format s "For more details on impl specific options, type:~%")
        (format s "   ~A help install impl~2%" cmd)
        (format s "Candidates impls for installation are:~%")
        (loop for i in (asdf:registered-systems)
              with len = #.(length "roswell.install.")
              when (and (string-equal "roswell.install." i :end2 (min (length i) len))
                        (not (eql (aref i (1- (length i))) #\+)))
                do (format s "~A~%" (subseq i len))))
      (let* ((impl (second argv))
             (fun (module "install" impl)))
        (install-impl impl nil nil (funcall fun :help)))))
