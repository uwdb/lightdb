import subprocess
from scannerpy import Database, DeviceType, Job, BulkJob
from scannerpy.stdlib import NetDescriptor
from scannerpy.stdlib.video import *
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

print video_path

with Database() as db:
    [input_table], _ = db.ingest_videos(
        [('video1', video_path)], force=True)

    frame = db.ops.FrameInput()
    sliced_frame = frame.slice()

    iframe = db.ops.identity(frame=sliced_frame, device=DeviceType.GPU)
    iframe = iframe.unslice()

    output = db.ops.Output(columns=[iframe])

    job = Job(op_args={
        frame: input_table.column('frame'),
        sliced_frame: db.partitioner.range(45, 45+60),
        output: input_table.name() + '_output'
    })
    bulk_job = BulkJob(output=output, jobs=[job])

    [output] = db.run(bulk_job, pipeline_instances_per_node=-1, tasks_in_queue_per_pu=10, force=True)
    #write_video(output.column('frame_output'), 30, 30)

    output.column('frame_output').save_mp4('out')
