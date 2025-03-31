set -e
MYPATH=$(realpath $0)
MYDIR=$(dirname $MYPATH)/..
OUTDIR=${MYDIR}/out
LOGDIR=${MYDIR}/log/mutex

ACQ_REL=${OUTDIR}/acq_rel_mutex
SEQ_CST=${OUTDIR}/seq_cst_mutex
if [[ ! -d ${LOGDIR} ]];
then
    mkdir ${LOGDIR};
fi

for i in 10 20 30;
do 
    acq_rel_log=$LOGDIR/acq_rel_${i}.log
    $ACQ_REL 5 $i 5 > $acq_rel_log;
    echo $acq_rel_log;

    seq_cst_log=$LOGDIR/seq_cst_${i}.log
    $SEQ_CST 5 $i 5 > $seq_cst_log;
    echo $seq_cst_log;
done
