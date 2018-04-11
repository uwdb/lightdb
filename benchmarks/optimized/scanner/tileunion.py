from scannerpy import Database, DeviceType, Job, BulkJob
from scannerpy.stdlib import NetDescriptor
import numpy as np
import cv2
import struct
import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/..')
sys.path.append('/opt/scanner/examples')
sys.path.append('/opt/scanner')
import util

video_path = '/app/foo.mp4' if len(sys.argv) <= 1 else sys.argv[1]
video_name = os.path.splitext(os.path.basename(video_path))[0]

with Database() as db:
    [input_table], _ = db.ingest_videos(
        [(video_name, video_path)], force=True)

    frame = db.ops.FrameInput()
    ctiles = [frame for _ in xrange(12)]
    joined = db.ops.Join(frame1=ctiles[0], frame2=ctiles[1], frame3=ctiles[2], frame4=ctiles[3],
                         frame5=ctiles[4], frame6=ctiles[5], frame7=ctiles[6], frame8=ctiles[7],
                         frame9=ctiles[8], frame10=ctiles[9], frame11=ctiles[10], frame12=ctiles[11])

    output = db.ops.Output(columns=[joined])

    job = Job(op_args={
        frame: input_table.column('frame'),
        output: input_table.name() + '_output'
    })
    bulk_job = BulkJob(output=output, jobs=[job])

    [output] = db.run(bulk_job, pipeline_instances_per_node=-1, tasks_in_queue_per_pu=10, force=True)
    output.column('frame_output').save_mp4(video_path + '_out')
