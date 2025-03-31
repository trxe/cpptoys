set -e
MYPATH=$(realpath $0)
MYDIR=$(dirname $MYPATH)/..
OUTDIR=${MYDIR}/out
LOGDIR=${MYDIR}/log/semaphore

ACQ_REL=${OUTDIR}/acq_rel_semaphore
SEQ_CST=${OUTDIR}/seq_cst_semaphore
RELAXED=${OUTDIR}/relaxed_semaphore
if [[ ! -d ${LOGDIR} ]];
then
    mkdir ${LOGDIR};
fi

for i in 50 100 200;
do 
    acq_rel_log=$LOGDIR/acq_rel_${i}.log
    $ACQ_REL $i > $acq_rel_log;
    echo $acq_rel_log;

    seq_cst_log=$LOGDIR/seq_cst_${i}.log
    $SEQ_CST $i > $seq_cst_log;
    echo $seq_cst_log;

    relaxed_log=$LOGDIR/relaxed_${i}.log
    $RELAXED $i > $relaxed_log;
    echo $relaxed_log;
done
