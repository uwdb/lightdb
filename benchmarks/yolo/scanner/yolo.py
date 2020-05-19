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
    bboxes = db.ops.Yolo(frame=frame, device=DeviceType.CPU)
    boxed_frame = db.ops.DrawBox(frame = frame, bboxes = bboxes)

    #boxed_frame.lossless()

    output = db.ops.Output(columns=[boxed_frame])

    job = Job(op_args={
        frame: input_table.column('frame'),
        output: input_table.name() + '_output'
    })
    bulk_job = BulkJob(output=output, jobs=[job])

    [output] = db.run(bulk_job, pipeline_instances_per_node=-1, tasks_in_queue_per_pu=10, force=True)
    output.column('frame').save_mp4(video_path + '_out')
