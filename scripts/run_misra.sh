#!/usr/bin/env bash
# Verifie que le code du BCM (dossier src/) respecte les regles MISRA C:2012.
# Le resultat est aussi enregistre dans misra-report.txt
cd "$(dirname "$0")/.."

cppcheck --addon=misra \
         --enable=warning,style,performance,portability \
         --std=c11 \
         --suppressions-list=misra_suppressions.txt \
         --error-exitcode=1 \
         -q -I include \
         src 2>&1 | tee misra-report.txt

if [ "${PIPESTATUS[0]}" -eq 0 ]; then
    echo "MISRA : aucune violation dans src/"
else
    echo "MISRA : des violations ont ete trouvees (voir ci-dessus)"
    exit 1
fi
